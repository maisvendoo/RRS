#!/usr/bin/env bash
#
# Сборка RRS под GNU/Linux без Docker.
#
# Что делает скрипт:
#   1. проверяет системные зависимости (Qt6, Vulkan, OpenAL, SFML, Lua и т.д.);
#   2. собирает из исходников зависимости, которых нет в репозиториях дистрибутива
#      (VulkanSceneGraph, vsgXchange, vsgImGui, KTX) в локальный префикс;
#      sol2 и SFML берутся пакетами дистрибутива, а из исходников собираются
#      только если системных CMake-конфигураций не нашлось;
#   3. конфигурирует и собирает RRS (CMake + Ninja);
#   4. при необходимости устанавливает результат: cmake --install (SDK, lua)
#      плюс копирование артефактов сборки (bin, lib, modules, plugins, cfg,
#      data) и маршрутов в каталог, указанный в --install;
#   5. по цели clean удаляет каталог сборки, артефакты сборки, каталог
#      установки (dist) и зависимости, скачанные этим скриптом.
#
# Зависимости, собранные из исходников, кладутся в .deps/install (по умолчанию),
# сборка — в build/. Повторный запуск не пересобирает уже готовые зависимости.
#
# Примеры:
#   ./scripts/build_linux.sh
#   ./scripts/build_linux.sh --install "$PWD/dist"
#   ./scripts/build_linux.sh --deps-only
#   ./scripts/build_linux.sh --skip-deps --build-dir build-release
#   ./scripts/build_linux.sh --check-only
#   ./scripts/build_linux.sh clean
#
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

# Копирование маршрутов в каталог установки
# shellcheck source=routes-copy.sh
. "${SCRIPT_DIR}/routes-copy.sh"
# shellcheck source=versions.env
. "${SCRIPT_DIR}/versions.env"

# Копирование артефактов сборки в каталог установки.
#
# cmake --install кладёт туда только SDK, lua-модули и маршруты: install(TARGETS)
# для bin/ и lib/ проект не объявляет. Кроме того, CMake задаёт
# EXECUTABLE_OUTPUT_PATH/LIBRARY_OUTPUT_PATH относительными путями
# ("../../bin", "../../lib"), поэтому артефакты ложатся в корень исходников,
# а не в каталог сборки. Копируем их руками — так же, как стадия RRS в
# Dockerfile.ubuntu26.04.
rrs_install_artifacts() {
  local dst_dir="$1"
  local name src

  # fonts/ и themes/ обязательны: FileSystem разрешает их как <корень>/fonts
  # и <корень>/themes, а GUI (vsgImGui, Qt) без них пишет
  # «Could not load font file!» и падает.
  for name in bin lib modules plugins cfg data fonts themes; do
    src="${ROOT_DIR}/${name}"
    if [ -d "$src" ]; then
      mkdir -p "${dst_dir}/${name}"
      cp -a "${src}/." "${dst_dir}/${name}/"
      log "Артефакты: ${name} -> ${dst_dir}/${name}"
    fi
  done

  # Каталоги времени выполнения, которые на Windows создаёт windeploy.bat:
  # logs/ (журналы) и screenshots/ (снимки экрана по F12). Каталог снимков
  # обязателен заранее: ScreenshotWriter пишет файл без mkpath(), и при
  # отсутствии каталога viewer сообщает «Fail to write screenchot».
  mkdir -p "${dst_dir}/logs" "${dst_dir}/screenshots"
  log "Каталоги времени выполнения: logs, screenshots"

  # README для того, кто распаковал комплект: команда установки зависимостей
  # и порядок запуска. Файл лежит в docs/ и копируется в корень установки.
  if [ -f "${ROOT_DIR}/docs/DIST_LINUX_README.md" ]; then
    cp -a "${ROOT_DIR}/docs/DIST_LINUX_README.md" "${dst_dir}/README.md"
    log "README: docs/DIST_LINUX_README.md -> ${dst_dir}/README.md"
  fi

  # RPATH бинарников — $ORIGIN/../lib, поэтому в каталог установки нужны и
  # библиотеки зависимостей (vsg, vsgXchange, vsgImGui, ktx).
  if [ -d "${DEPS_PREFIX}/lib" ]; then
    mkdir -p "${dst_dir}/lib"
    cp -a "${DEPS_PREFIX}/lib/." "${dst_dir}/lib/"
    log "Библиотеки зависимостей: ${DEPS_PREFIX}/lib -> ${dst_dir}/lib"
  fi
}

# --- Значения по умолчанию (переопределяются переменными окружения) --------
DEPS_PREFIX="${RRS_DEPS_PREFIX:-${ROOT_DIR}/.deps/install}"
DEPS_SRC_DIR="${RRS_DEPS_SRC_DIR:-${ROOT_DIR}/.deps/src}"
BUILD_DIR="${RRS_BUILD_DIR:-${ROOT_DIR}/build}"
BUILD_TYPE="${RRS_BUILD_TYPE:-Release}"
JOBS="${RRS_JOBS:-$(nproc)}"
EXTRA_PREFIX_PATH="${RRS_EXTRA_PREFIX_PATH:-}"
# Маршруты: каталог-источник и что копировать в каталог установки
# (all | none | список имён через запятую)
ROUTES_SRC="${RRS_ROUTES_SRC:-${ROOT_DIR}/routes}"
ROUTES="${RRS_ROUTES:-all}"

# # Версии зависимостей, собираемых из исходников
# VSG_VERSION="${VSG_VERSION:-v1.1.16}"
# VSGXCHANGE_VERSION="${VSGXCHANGE_VERSION:-v1.1.14}"
# VSGIMGUI_REPO="${VSGIMGUI_REPO:-https://github.com/maisvendoo/vsgImGui.git}"
# VSGIMGUI_VERSION="${VSGIMGUI_VERSION:-master-RRS}"
# SOL2_REPO="${SOL2_REPO:-https://github.com/maisvendoo/sol2.git}"
# SOL2_VERSION="${SOL2_VERSION:-c1f95a773c6f8f4fde8ca3efe872e7286afe4444}"
# # KTX: код RRS (tools/dmd2gltf, tools/texture-compressor) использует поля
# # ktxBasisParams::codec и ::etc1sCompressionLevel — их нет ни в одном релизе
# # 4.x (в 4.4.2 они переименованы), поэтому берём конкретный коммит master.
# KTX_VERSION="${KTX_VERSION:-983695c105618462b8e96458b378222e99faec98}"
# # SFML: в Убунту 26.04 есть пакет libsfml-dev 3.0, он и используется;
# # из исходников SFML собирается только если системного конфига нет.
# SFML_VERSION="${SFML_VERSION:-3.1.0}"

DO_DEPS=1
DEPS_ONLY=0
REBUILD_DEPS=0
DO_CLEAN=0
CLEAN_ALL=0
DO_INSTALL=0
INSTALL_DIR=""
CHECK_ONLY=0
GENERATOR=""
EXTRA_CMAKE_ARGS=()

# --- Полная очистка (цель clean) ------------------------------------------

# Удаление файла или каталога с проверками: нельзя удалить корень файловой
# системы или корень репозитория.
rrs_remove_path() {
  local path="$1" label="$2" abs size

  if [[ -z "$path" ]]; then
    warn "${label}: путь не задан, пропуск"
    return 0
  fi

  abs="$(realpath -m -- "$path")"
  if [[ "$abs" == "/" || "$abs" == "${ROOT_DIR}" || -z "$abs" ]]; then
    warn "${label}: отказ удалять '${abs}'"
    return 0
  fi
  if [[ ! -e "$abs" ]]; then
    echo "  [НЕТ   ] ${label}: ${abs}"
    return 0
  fi

  size="$(du -sh -- "$abs" 2>/dev/null | cut -f1)"
  rm -rf -- "$abs"
  echo "  [УДАЛЕН] ${label}: ${abs} (${size:-?})"
}

# Полная очистка: каталог сборки, артефакты сборки в корне исходников
# (bin, lib, modules, plugins), каталог установки (по умолчанию dist) и
# зависимости, скачанные и собранные этим скриптом (.deps).
# cfg/ и data/ не трогаются: они входят в репозиторий.
clean_all() {
  local dist_dir="${INSTALL_DIR:-${ROOT_DIR}/dist}"

  log "Полная очистка: сборка, артефакты, каталог установки, зависимости"
  echo "  Корень репозитория: ${ROOT_DIR}"

  # Порядок важен: сначала вложенные каталоги зависимостей, затем .deps
  rrs_remove_path "${BUILD_DIR}" "каталог сборки"
  rrs_remove_path "${ROOT_DIR}/bin" "исполняемые файлы"
  rrs_remove_path "${ROOT_DIR}/lib" "библиотеки"
  rrs_remove_path "${ROOT_DIR}/modules" "подключаемые модули"
  rrs_remove_path "${ROOT_DIR}/plugins" "плагины"
  rrs_remove_path "${dist_dir}" "каталог установки"
  rrs_remove_path "${DEPS_SRC_DIR}" "исходники зависимостей"
  rrs_remove_path "${DEPS_PREFIX}" "установленные зависимости"
  rrs_remove_path "${ROOT_DIR}/.deps" "каталог зависимостей"

  log "Очистка завершена"
  echo "  Для повторной сборки: ${0} --install \"\$PWD/dist\""
  echo "  Зависимости будут собраны заново (VulkanSceneGraph и прочее, ~10-20 мин)"
}

usage() {
  cat <<EOF
Использование: $(basename "$0") [параметры]

Параметры:
  -t, --build-type ТИП   Release | RelWithDebInfo | Debug | MinSizeRel (по умолчанию: ${BUILD_TYPE})
  -j, --jobs N           Число параллельных задач (по умолчанию: ${JOBS})
  -b, --build-dir КАТАЛОГ Каталог сборки (по умолчанию: ${BUILD_DIR})
      --deps-prefix КАТАЛОГ Префикс для зависимостей (по умолчанию: ${DEPS_PREFIX})
      --install КАТАЛОГ   Установить в каталог: cmake --install + артефакты сборки
                          (bin, lib, modules, plugins, cfg, data, fonts, themes) и маршруты
      --deps-only         Только собрать зависимости, без сборки RRS
      --skip-deps         Не собирать зависимости (считать, что они уже есть)
      --rebuild-deps      Пересобрать зависимости даже если они уже установлены
      --clean             Удалить каталог сборки перед конфигурированием
      clean               Полная очистка: удалить каталог сборки, артефакты
                          (bin, lib, modules, plugins), каталог установки
                          (по умолчанию dist) и зависимости (.deps).
                          Сборка не выполняется. Каталог установки можно
                          задать параметром --install, зависимости —
                          параметром --deps-prefix.
      --generator ГЕНЕРАТОР  Генератор CMake (по умолчанию: Ninja, иначе Unix Makefiles)
      --check-only        Только проверить наличие зависимостей
      --routes СПИСОК    Какие маршруты положить в <каталог установки>/routes:
                         all (по умолчанию) | none | имена через запятую
      --no-routes        Не копировать маршруты
  -h, --help             Показать эту справку

Все параметры после «--» передаются в cmake без изменений.

Переменные окружения: RRS_DEPS_PREFIX, RRS_DEPS_SRC_DIR, RRS_BUILD_DIR,
RRS_BUILD_TYPE, RRS_JOBS, RRS_EXTRA_PREFIX_PATH, RRS_ROUTES_SRC, RRS_ROUTES,
VSG_VERSION, VSGXCHANGE_VERSION, VSGIMGUI_VERSION, SOL2_VERSION, KTX_VERSION.
EOF
}

log() { echo "==> $*"; }
warn() { echo "ВНИМАНИЕ: $*" >&2; }
die() {
  echo "Ошибка: $*" >&2
  exit 1
}

# --- Разбор аргументов ----------------------------------------------------
while [[ $# -gt 0 ]]; do
  case "$1" in
  -t | --build-type)
    BUILD_TYPE="$2"
    shift 2
    ;;
  -j | --jobs)
    JOBS="$2"
    shift 2
    ;;
  -b | --build-dir)
    BUILD_DIR="$2"
    shift 2
    ;;
  --deps-prefix)
    DEPS_PREFIX="$2"
    shift 2
    ;;
  --install)
    INSTALL_DIR="$2"
    DO_INSTALL=1
    shift 2
    ;;
  --deps-only)
    DEPS_ONLY=1
    DO_INSTALL=0
    shift
    ;;
  --skip-deps)
    DO_DEPS=0
    shift
    ;;
  --rebuild-deps)
    REBUILD_DEPS=1
    shift
    ;;
  --clean)
    DO_CLEAN=1
    shift
    ;;
  clean)
    CLEAN_ALL=1
    shift
    ;;
  --generator)
    GENERATOR="$2"
    shift 2
    ;;
  --check-only)
    CHECK_ONLY=1
    shift
    ;;
  --routes)
    ROUTES="$2"
    shift 2
    ;;
  --no-routes)
    ROUTES="none"
    shift
    ;;
  -h | --help)
    usage
    exit 0
    ;;
  --)
    shift
    EXTRA_CMAKE_ARGS=("$@")
    break
    ;;
  *) die "неизвестный параметр: $1 (см. --help)" ;;
  esac
done

# Цель clean: полная очистка, сборка не выполняется
if [[ "$CLEAN_ALL" -eq 1 ]]; then
  clean_all
  exit 0
fi

if [[ "$CHECK_ONLY" -eq 0 ]]; then
  case "$BUILD_TYPE" in
  Release | RelWithDebInfo | Debug | MinSizeRel) ;;
  *) die "неверный тип сборки '${BUILD_TYPE}'" ;;
  esac
fi

# --- Поиск CMake-конфигураций зависимостей --------------------------------
CONFIG_SEARCH_DIRS=(
  "${DEPS_PREFIX}/lib/cmake"
  "${DEPS_PREFIX}/lib64/cmake"
  "${DEPS_PREFIX}/share/cmake"
  "/usr/local/lib/cmake"
  "/usr/lib/cmake"
  "/usr/share/cmake"
  "/usr/lib/x86_64-linux-gnu/cmake"
)

have_cmake_config() {
  local cfg="$1" dir
  for dir in "${CONFIG_SEARCH_DIRS[@]}"; do
    [[ -f "${dir}/${cfg}" ]] && return 0
  done
  return 1
}

have_pkg_config() {
  command -v pkg-config >/dev/null 2>&1 && pkg-config --exists "$1" 2>/dev/null
}

# --- Проверка окружения ---------------------------------------------------
check_environment() {
  local missing=0 cmd cmake_version

  log "Проверка инструментов и зависимостей (префикс: ${DEPS_PREFIX})"

  for cmd in cmake ninja g++ git pkg-config; do
    if command -v "$cmd" >/dev/null 2>&1; then
      echo "  [  OK  ] ${cmd}: $(command -v "$cmd")"
    else
      echo "  [НЕТ   ] ${cmd}"
      missing=1
    fi
  done

  if command -v cmake >/dev/null 2>&1; then
    cmake_version="$(cmake --version | sed -n '1s/.*[[:space:]]version[[:space:]]*//p')"
    if printf '%s\n%s\n' "3.29" "$cmake_version" | sort -V -C 2>/dev/null; then
      echo "  [  OK  ] cmake ${cmake_version} (>= 3.29)"
    else
      echo "  [ВЕРСИЯ] cmake ${cmake_version}, требуется >= 3.29"
      missing=1
    fi
  fi

  # Системные библиотеки (в Убунту/Debian ставятся пакетами)
  if have_cmake_config "Qt6/Qt6Config.cmake"; then
    echo "  [  OK  ] Qt6"
    # Без Qt6LinguistTools не собирается launcher (find_package Qt6 ... LinguistTools)
    if ! have_cmake_config "Qt6LinguistTools/Qt6LinguistToolsConfig.cmake"; then
      echo "  [НЕТ   ] Qt6LinguistTools (пакет qt6-tools-dev)"
      missing=1
    fi
  else
    echo "  [НЕТ   ] Qt6 (qt6-base-dev)"
    missing=1
  fi
  if have_cmake_config "Qt6Pdf/Qt6PdfConfig.cmake"; then
    echo "  [  OK  ] Qt6Pdf"
  else
    echo "  [НЕТ   ] Qt6Pdf (qt6-pdf-dev)"
    missing=1
  fi
  if have_cmake_config "Qt6SerialBus/Qt6SerialBusConfig.cmake"; then
    echo "  [  OK  ] Qt6SerialBus"
  else
    echo "  [НЕТ   ] Qt6SerialBus (qt6-serialbus-dev)"
    missing=1
  fi

  for spec in "openal:OpenAL (libopenal-dev)" \
    "assimp:assimp (libassimp-dev)" "freetype2:freetype (libfreetype-dev)" \
    "lua5.4:Lua 5.4 (liblua5.4-dev)" "vulkan:Vulkan (libvulkan-dev)"; do
    local pc="${spec%%:*}" label="${spec#*:}"
    if have_pkg_config "$pc"; then
      echo "  [  OK  ] ${label}"
    else
      echo "  [НЕТ   ] ${label}"
      missing=1
    fi
  done

  # glslang нужен VulkanSceneGraph: без него CMake-конфиг glslang не находится,
  # vsg отключает ShaderCompiler (VSG_SUPPORTS_ShaderCompiler) и собранные
  # симулятор/вьюер падают с «VulkanSceneGraph not compiled with GLSLang».
  if have_cmake_config "glslang/glslang-config.cmake"; then
    echo "  [  OK  ] glslang"
  else
    echo "  [НЕТ   ] glslang (пакет glslang-dev) — без него не собираются шейдеры"
    missing=1
  fi

  # SFML 3.x: в Убунту 26.04 есть пакет libsfml-dev 3.0; если конфигурации
  # нет, скрипт собирает SFML из исходников.
  if have_cmake_config "SFML/SFMLConfig.cmake"; then
    if [[ -f "${DEPS_PREFIX}/lib/cmake/SFML/SFMLConfig.cmake" ]]; then
      if dep_up_to_date "${DEPS_SRC_DIR}/SFML" "$SFML_VERSION" "$SFML_REPO"; then
        echo "  [  OK  ] SFML (${SFML_VERSION})"
      else
        echo "  [ВЕРСИЯ] SFML (будет обновлён до ${SFML_VERSION})"
      fi
    else
      echo "  [  OK  ] SFML >= 3 (системный)"
    fi
  else
    echo "  [НЕТ   ] SFML >= 3 (будет собран из исходников: ${SFML_VERSION})"
  fi

  local cfg
  for spec in "vsg/vsgConfig.cmake|VulkanSceneGraph|${DEPS_SRC_DIR}/vsg|${VSG_VERSION}|${VSG_REPO}" \
    "vsgXchange/vsgXchangeConfig.cmake|vsgXchange|${DEPS_SRC_DIR}/vsgXchange|${VSGXCHANGE_VERSION}|${VSGXCHANGE_REPO}" \
    "vsgImGui/vsgImGuiConfig.cmake|vsgImGui|${DEPS_SRC_DIR}/vsgImGui|${VSGIMGUI_VERSION}|${VSGIMGUI_REPO}" \
    "sol2/sol2-config.cmake|sol2|${DEPS_SRC_DIR}/sol2|${SOL2_VERSION}|${SOL2_REPO}" \
    "ktx/KtxConfig.cmake|KTX|${DEPS_SRC_DIR}/KTX-Software|${KTX_VERSION}|${KTX_REPO}"; do
    cfg="${spec%%:*}"
    IFS='|' read -r cfg label src version repo <<<"$spec"
    if have_cmake_config "$cfg"; then
      if [[ -f "${DEPS_PREFIX}/lib/cmake/${cfg}" ]]; then
        if dep_up_to_date "$src" "$version" "$repo"; then
          echo "  [  OK  ] ${label} (${version})"
        else
          echo "  [ВЕРСИЯ] ${label} (будет обновлён до ${version})"
        fi
      else
        echo "  [  ОК  ] ${label} (системный)"
      fi
    else
      echo "  [НЕТ   ] ${spec#*:} (можно собрать: --deps-only)"
    fi
  done

  if [[ "$missing" -ne 0 ]]; then
    echo
    warn "обнаружены отсутствующие системные зависимости."
    cat <<'HINT'
Установите их пакетами дистрибутива, например для Убунту/Debian:
  sudo apt install build-essential ninja-build pkg-config git cmake \
      qt6-base-dev qt6-pdf-dev qt6-serialbus-dev qt6-l10n-tools qt6-tools-dev \
      libvulkan-dev glslang-dev libopenal-dev liblua5.4-dev \
      libassimp-dev libfreetype-dev zlib1g-dev libpng-dev \
      libx11-dev libxcb1-dev libxkbcommon-x11-dev libwayland-dev \
      libxrandr-dev libxcursor-dev libxi-dev libudev-dev \
      libsfml-dev sol2-dev
VulkanSceneGraph, vsgXchange, vsgImGui и KTX в репозиториях Убунту нет —
скрипт собирает их из исходников (см. --deps-only).
HINT
    if [[ "$CHECK_ONLY" -eq 1 ]]; then
      exit 1
    fi
    die "установите перечисленные пакеты и повторите запуск"
  fi
}

# --- Сборка зависимостей --------------------------------------------------
dep_up_to_date() {
  local dir="$1" ref="$2" repo="$3"
  [[ -d "${dir}/.git" ]] || return 1

  local local_sha
  local_sha="$(git -C "${dir}" rev-parse HEAD 2>/dev/null || true)"
  [[ -n "$local_sha" ]] || return 1

  if [[ "$ref" =~ ^[0-9a-fA-F]{40}$ ]]; then
    [[ "$local_sha" == "$ref" ]]
    return $?
  fi

  local remote_out remote_sha
  remote_out="$(git ls-remote "$repo" "refs/heads/${ref}" "refs/tags/${ref}" "refs/tags/${ref}^{}" 2>/dev/null || true)"

  if [[ -n "$remote_out" ]]; then
    remote_sha="$(printf '%s\n' "$remote_out" | awk '/\^\{\}$/ {print $1; found=1} END {if (!found) exit 1}' || printf '%s\n' "$remote_out" | awk 'NR==1 {print $1}')"
    [[ "$local_sha" == "$remote_sha" ]]
    return $?
  fi

  local local_tag local_branch
  local_tag="$(git -C "$dir" describe --tags --exact-match 2>/dev/null || true)"
  local_branch="$(git -C "$dir" branch --show-current 2>/dev/null || true)"
  [[ -n "$local_tag" && "$local_tag" == "$ref" || -n "$local_branch" && "$ref" ]]
}

dep_ready() {
  # $1 - путь к CMake-конфигу зависимости
  # $2 - путь к скачанному репозиторию зависимости
  # $3 - необходимая версия зависимости
  # $4 - репозиторий зависимости
  local cfg="$1" src_dir="${2:-}" version="${3:-}" repo="${4:-}"

  [[ "$REBUILD_DEPS" -eq 0 && -f "$cfg" ]] || return 1
  dep_up_to_date "$src_dir" "$version" "$repo"
}

dep_clone() {
  # $1 — каталог, $2 — репозиторий, остальное — аргументы git clone
  local dir="$1" repo="$2"
  shift 2
  #if [[ -d "${dir}/.git" ]]; then
  #    return 0
  #fi

  # --branch не принимает SHA-1: git завершается с ошибкой
  # «Remote branch <sha> not found in upstream origin». Поэтому
  # закреплённый коммит клонируем без --branch и переключаемся на него
  # отдельно (fetch + checkout FETCH_HEAD).
  local ref=""
  local -a args=()
  while [[ $# -gt 0 ]]; do
    case "$1" in
    --branch)
      ref="${2:-}"
      shift 2
      ;;
    *)
      args+=("$1")
      shift
      ;;
    esac
  done

  if [[ -n "$ref" ]] && dep_up_to_date "$dir" "$ref" "$repo"; then
    return 0
  fi

  rm -rf "$dir"

  if [[ "${ref}" =~ ^[0-9a-fA-F]{40}$ ]]; then
    local -a clone_args=()
    local arg
    for arg in ${args[@]+"${args[@]}"}; do
      # Подмодули закреплённого коммита подтягиваем после checkout
      case "${arg}" in
      --recurse-submodules | --shallow-submodules) ;;
      *) clone_args+=("${arg}") ;;
      esac
    done
    log "git clone ${repo} (коммит ${ref}) -> ${dir}"
    git clone ${clone_args[@]+"${clone_args[@]}"} "${repo}" "${dir}"
    git -C "${dir}" fetch --depth 1 origin "${ref}"
    git -C "${dir}" checkout -q FETCH_HEAD
    if [[ -f "${dir}/.gitmodules" ]]; then
      git -C "${dir}" submodule update --init --recursive --depth 1
    fi
    return 0
  fi

  local -a branch_args=()
  if [[ -n "${ref}" ]]; then
    branch_args=(--branch "${ref}")
  fi
  log "git clone ${repo}${ref:+ (${ref})} -> ${dir}"
  git clone ${args[@]+"${args[@]}"} \
    ${branch_args[@]+"${branch_args[@]}"} "${repo}" "${dir}"
}

cmake_dep() {
  # Общие аргументы для конфигурирования зависимостей
  cmake -S "$1" -B "$1/build" -G "${GENERATOR_EFFECTIVE}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="${DEPS_PREFIX}" \
    -DCMAKE_PREFIX_PATH="${DEPS_PREFIX}" \
    -DBUILD_SHARED_LIBS=ON
  cmake --build "$1/build" -j "$JOBS"
  cmake --install "$1/build"
}

build_vsg() {
  local src="${DEPS_SRC_DIR}/vsg"
  if dep_ready "${DEPS_PREFIX}/lib/cmake/vsg/vsgConfig.cmake" "$src" "${VSG_VERSION}" "$VSG_REPO"; then
    log "vsg уже установлен — пропуск"
    return 0
  fi
  dep_clone "$src" "$VSG_REPO" \
    --depth 1 --branch "${VSG_VERSION}"
  log "Сборка vsg (${VSG_VERSION})"
  cmake -S "$src" -B "$src/build" -G "${GENERATOR_EFFECTIVE}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="${DEPS_PREFIX}" \
    -DCMAKE_PREFIX_PATH="${DEPS_PREFIX}" \
    -DBUILD_SHARED_LIBS=ON \
    -DVSG_SUPPORTS_ShaderCompiler=ON \
    -DVSG_SUPPORTS_ShaderOptimizer=OFF \
    -DVSG_SUPPORTS_Windowing=ON
  cmake --build "$src/build" -j "$JOBS"
  cmake --install "$src/build"
}

build_vsgxchange() {
  local src="${DEPS_SRC_DIR}/vsgXchange"
  if dep_ready "${DEPS_PREFIX}/lib/cmake/vsgXchange/vsgXchangeConfig.cmake" "$src" "$VSGXCHANGE_VERSION" "$VSGXCHANGE_REPO"; then
    log "vsgXchange уже установлен — пропуск"
    return 0
  fi
  dep_clone "$src" "$VSGXCHANGE_REPO" \
    --depth 1 --branch "${VSGXCHANGE_VERSION}"
  log "Сборка vsgXchange (${VSGXCHANGE_VERSION})"
  cmake_dep "$src"
}

build_vsgimgui() {
  local src="${DEPS_SRC_DIR}/vsgImGui"
  if dep_ready "${DEPS_PREFIX}/lib/cmake/vsgImGui/vsgImGuiConfig.cmake" "$src" "$VSGIMGUI_VERSION" "$VSGIMGUI_REPO"; then
    log "vsgImGui уже установлен — пропуск"
    return 0
  fi
  dep_clone "$src" "${VSGIMGUI_REPO}" \
    --depth 1 --recursive --shallow-submodules --branch "${VSGIMGUI_VERSION}"
  log "Сборка vsgImGui (${VSGIMGUI_VERSION})"
  cmake_dep "$src"
}

build_sol2() {
  if have_cmake_config "sol2/sol2-config.cmake"; then
    log "sol2 есть в системе — сборка из исходников не нужна"
    return 0
  fi
  local src="${DEPS_SRC_DIR}/sol2"
  if dep_ready "${DEPS_PREFIX}/share/cmake/sol2/sol2-config.cmake" "$src" "$SOL2_VERSION" "$SOL2_REPO"; then
    log "sol2 уже установлен — пропуск"
    return 0
  fi
  dep_clone "$src" "${SOL2_REPO}"
  if [[ -n "${SOL2_VERSION}" ]]; then
    git -C "$src" checkout "${SOL2_VERSION}"
  fi
  log "Сборка sol2 (${SOL2_VERSION})"
  cmake -S "$src" -B "$src/build" -G "${GENERATOR_EFFECTIVE}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="${DEPS_PREFIX}" \
    -DSOL2_TESTS=OFF \
    -DSOL2_EXAMPLES=OFF
  cmake --build "$src/build" -j "$JOBS"
  cmake --install "$src/build"
}

build_sfml() {
  if have_cmake_config "SFML/SFMLConfig.cmake"; then
    log "SFML >= 3 есть в системе — сборка из исходников не нужна"
    return 0
  fi
  local src="${DEPS_SRC_DIR}/SFML"
  if dep_ready "${DEPS_PREFIX}/lib/cmake/SFML/SFMLConfig.cmake" "$src" "$SFML_VERSION" "$SFML_REPO"; then
    log "SFML уже установлен — пропуск"
    return 0
  fi
  dep_clone "$src" "$SFML_REPO" \
    --depth 1 --branch "${SFML_VERSION}"
  log "Сборка SFML (${SFML_VERSION}, модули System и Window)"
  cmake -S "$src" -B "$src/build" -G "${GENERATOR_EFFECTIVE}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="${DEPS_PREFIX}" \
    -DCMAKE_PREFIX_PATH="${DEPS_PREFIX}" \
    -DBUILD_SHARED_LIBS=ON \
    -DSFML_BUILD_SYSTEM=ON \
    -DSFML_BUILD_WINDOW=ON \
    -DSFML_BUILD_GRAPHICS=OFF \
    -DSFML_BUILD_AUDIO=OFF \
    -DSFML_BUILD_NETWORK=OFF \
    -DSFML_BUILD_EXAMPLES=OFF \
    -DSFML_BUILD_TEST_SUITE=OFF \
    -DSFML_BUILD_DOC=OFF
  cmake --build "$src/build" -j "$JOBS"
  cmake --install "$src/build"
}

build_ktx() {
  # KTX ставит конфиг в lib/cmake/ktx/KtxConfig.cmake (именно такое
  # написание, поэтому проверку делаем по фактическому пути)
  local src="${DEPS_SRC_DIR}/KTX-Software"
  if dep_ready "${DEPS_PREFIX}/lib/cmake/ktx/KtxConfig.cmake" "$src" "$KTX_VERSION" "$KTX_REPO"; then
    log "KTX уже установлен — пропуск"
    return 0
  fi
  dep_clone "$src" "$KTX_REPO" \
    --depth 1 --recurse-submodules --shallow-submodules --branch "${KTX_VERSION}"
  log "Сборка KTX (${KTX_VERSION})"
  cmake -S "$src" -B "$src/build" -G "${GENERATOR_EFFECTIVE}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="${DEPS_PREFIX}" \
    -DCMAKE_PREFIX_PATH="${DEPS_PREFIX}" \
    -DBUILD_SHARED_LIBS=ON \
    -DKTX_FEATURE_TESTS=OFF \
    -DKTX_FEATURE_TOOLS=OFF \
    -DKTX_FEATURE_DOC=OFF \
    -DKTX_FEATURE_JNI=OFF \
    -DKTX_FEATURE_PY=OFF \
    -DKTX_FEATURE_VK_UPLOAD=ON \
    -DKTX_FEATURE_GL_UPLOAD=OFF
  cmake --build "$src/build" -j "$JOBS"
  cmake --install "$src/build"
}

build_dependencies() {
  mkdir -p "${DEPS_PREFIX}" "${DEPS_SRC_DIR}"
  build_vsg
  build_vsgxchange
  build_vsgimgui
  build_sol2
  build_sfml
  build_ktx
  log "Зависимости установлены в ${DEPS_PREFIX}"
}

# --- Основной сценарий ----------------------------------------------------
check_environment

if [[ "$CHECK_ONLY" -eq 1 ]]; then
  log "Проверка завершена"
  exit 0
fi

# Генератор по умолчанию: Ninja, если он установлен
GENERATOR_EFFECTIVE="${GENERATOR}"
if [[ -z "$GENERATOR_EFFECTIVE" ]]; then
  if command -v ninja >/dev/null 2>&1; then
    GENERATOR_EFFECTIVE="Ninja"
  else
    GENERATOR_EFFECTIVE="Unix Makefiles"
  fi
fi

# CMake 4.x убрал совместимость с cmake_minimum_required < 3.5,
# а часть конфигураций зависимостей (например OpenAL) составлена
# по старым правилам и требует эмуляции политики CMake 3.5
CMAKE_COMPAT_ARGS=()
CMAKE_MAJOR="$(cmake --version 2>/dev/null | sed -n '1s/^[^0-9]*\([0-9][0-9]*\).*/\1/p')"
if [[ -n "$CMAKE_MAJOR" && "$CMAKE_MAJOR" -ge 4 ]]; then
  CMAKE_COMPAT_ARGS=(-DCMAKE_POLICY_VERSION_MINIMUM=3.5)
  log "CMake ${CMAKE_MAJOR}.x: включена эмуляция политики CMake 3.5"
fi

if [[ "$DO_DEPS" -eq 1 ]]; then
  build_dependencies
fi

if [[ "$DEPS_ONLY" -eq 1 ]]; then
  log "Зависимости собраны, сборка RRS пропущена (--deps-only)"
  exit 0
fi

if [[ "$DO_CLEAN" -eq 1 && -d "$BUILD_DIR" ]]; then
  log "Удаление каталога сборки ${BUILD_DIR}"
  rm -rf "$BUILD_DIR"
fi

PREFIX_PATH="${DEPS_PREFIX}"
if [[ -n "$EXTRA_PREFIX_PATH" ]]; then
  PREFIX_PATH="${PREFIX_PATH};${EXTRA_PREFIX_PATH}"
fi

CONFIGURE_ARGS=(
  -S "${ROOT_DIR}"
  -B "${BUILD_DIR}"
  -G "${GENERATOR_EFFECTIVE}"
  -DCMAKE_BUILD_TYPE="${BUILD_TYPE}"
  -DCMAKE_PREFIX_PATH="${PREFIX_PATH}"
  -DCLIENT_BUILD=ON
  -DBUILD_TESTING=OFF
)
CONFIGURE_ARGS+=(${CMAKE_COMPAT_ARGS[@]+"${CMAKE_COMPAT_ARGS[@]}"})
if [[ -n "$INSTALL_DIR" ]]; then
  CONFIGURE_ARGS+=("-DCMAKE_INSTALL_PREFIX=${INSTALL_DIR}")
fi

log "Конфигурирование RRS (${BUILD_TYPE}, генератор ${GENERATOR_EFFECTIVE})"
cmake "${CONFIGURE_ARGS[@]}" ${EXTRA_CMAKE_ARGS[@]+"${EXTRA_CMAKE_ARGS[@]}"}

log "Сборка RRS (${JOBS} задач)"
cmake --build "${BUILD_DIR}" -j "${JOBS}"

# Проект задаёт EXECUTABLE_OUTPUT_PATH/LIBRARY_OUTPUT_PATH относительными
# путями ("../../bin", "../../lib"), поэтому CMake разрешает их относительно
# каталога сборки, и артефакты ложатся не в ${BUILD_DIR}/bin, а в
# ${ROOT_DIR}/bin, ${ROOT_DIR}/lib, ${ROOT_DIR}/modules, ${ROOT_DIR}/plugins.
ARTIFACT_DIR="${ROOT_DIR}"

if [[ "$DO_INSTALL" -eq 1 ]]; then
  log "Установка в ${INSTALL_DIR}"
  cmake --install "${BUILD_DIR}"
  rrs_install_artifacts "${INSTALL_DIR}"
  # Маршруты нужны рядом с bin/ (лаунчер и симулятор ищут их в <корень>/routes)
  rrs_copy_routes "${ROUTES_SRC}" "${INSTALL_DIR}/routes" "${ROUTES}"
fi

echo
log "Готово: сборка в ${BUILD_DIR}"
echo "    Исполняемые файлы: ${ARTIFACT_DIR}/bin"
echo "    Библиотеки:        ${ARTIFACT_DIR}/lib"
echo "    Подключаемые модули: ${ARTIFACT_DIR}/modules"
echo "    Плагины:           ${ARTIFACT_DIR}/plugins"
if [[ "$DO_INSTALL" -eq 1 ]]; then
  echo "    Каталог установки:  ${INSTALL_DIR}"
  echo "                        bin, lib, modules, plugins, cfg, data, fonts, themes, routes, sdk,"
  echo "                        logs, screenshots"
else
  echo "    Установка:         ${0} --install <каталог>"
fi
echo "    Полная очистка:    ${0} clean"
