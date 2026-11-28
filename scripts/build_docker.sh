#!/usr/bin/env bash
#
# Сборка RRS в Docker-контейнере на базе Убунту 26.04 LTS.
#
# Скрипт выполняет два шага:
#   1. docker build  — собирает образ (зависимости + сам RRS);
#   2. docker create/cp/rm — копирует результат сборки (/opt/rrs-dist) в
#      локальный каталог и удаляет контейнер. Контейнер не запускается
#      (кроме случая, когда нужно исправить владельца файлов прошлых запусков),
#      поэтому после завершения ничего не остаётся ни в списке контейнеров,
#      ни в списке образов, кроме самого образа сборки.
#
# Результат: bin, lib, modules, plugins, sdk, routes (маршруты).
#
# Маршруты не входят в образ (это сотни мегабайт, не нужные для сборки),
# поэтому после выгрузки артефактов каталоги из routes/ копируются
# в <результат>/routes — см. scripts/routes-copy.sh.
#
# Примеры:
#   ./scripts/build_docker.sh
#   ./scripts/build_docker.sh --jobs 16 --output dist
#   ./scripts/build_docker.sh --build-only             # только образ
#   ./scripts/build_docker.sh --no-build --output dist # только выгрузка артефактов
#   ./scripts/build_docker.sh --routes experimental-polygon_v2.0_ktx
#
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

# Копирование маршрутов в дерево результата
# shellcheck source=routes-copy.sh
. "${SCRIPT_DIR}/routes-copy.sh"
# shellcheck source=versions.env
. "${SCRIPT_DIR}/versions.env"

# --- Значения по умолчанию (переопределяются переменными окружения) --------
IMAGE="${RRS_DOCKER_IMAGE:-rrs-build:ubuntu-26.04}"
OUTPUT_DIR="${RRS_DOCKER_OUTPUT:-${ROOT_DIR}/dist}"
BUILD_TYPE="${RRS_BUILD_TYPE:-Release}"
JOBS="${RRS_JOBS:-$(nproc)}"
INSTALL_PREFIX="${RRS_ARTIFACT_PREFIX:-/opt/rrs-dist}"
# Маршруты: каталог-источник и что копировать (all | none | список через запятую)
ROUTES_SRC="${RRS_ROUTES_SRC:-${ROOT_DIR}/routes}"
ROUTES="${RRS_ROUTES:-all}"

# Версии зависимостей, собираемых из исходников внутри образа
#VSG_VERSION="${VSG_VERSION:-v1.1.16}"
#VSGXCHANGE_VERSION="${VSGXCHANGE_VERSION:-v1.1.14}"
#VSGIMGUI_REPO="${VSGIMGUI_REPO:-https://github.com/maisvendoo/vsgImGui.git}"
#VSGIMGUI_VERSION="${VSGIMGUI_VERSION:-master-RRS}"
# KTX: пакета нет в Убунту 26.04, а релизы KTX не содержат ktxBasisParams::codec,
# который нужен коду RRS. Поэтому собираем конкретный коммит ветки main.
#KTX_VERSION="${KTX_VERSION:-983695c105618462b8e96458b378222e99faec98}"

DOCKER_BIN="${DOCKER:-docker}"

NO_CACHE=0
PULL=0
DO_BUILD=1
EXPORT=1
DEPS_ONLY=0

usage() {
    cat <<EOF
Использование: $(basename "$0") [параметры]

Параметры:
  -i, --image ИМЯ        Имя образа (по умолчанию: ${IMAGE})
  -o, --output КАТАЛОГ   Куда положить результат сборки (по умолчанию: ${OUTPUT_DIR})
  -t, --build-type ТИП   Тип сборки: Release | RelWithDebInfo | Debug (по умолчанию: ${BUILD_TYPE})
      --deps-only        Собрать образ только с зависимостями
  -j, --jobs N           Число параллельных задач (по умолчанию: ${JOBS})
      --build-only       Только собрать образ, без выгрузки артефактов
      --no-build         Не собирать образ, только выгрузить артефакты
      --no-cache         Собирать образ без кеша
      --pull             Обновить базовый образ перед сборкой
      --routes СПИСОК    Какие маршруты положить в <результат>/routes:
                         all (по умолчанию) | none | имена через запятую
      --no-routes        Не копировать маршруты
  -h, --help             Показать эту справку

Переменные окружения: RRS_DOCKER_IMAGE, RRS_DOCKER_OUTPUT, RRS_DOCKERFILE,
RRS_BUILD_TYPE, RRS_JOBS, RRS_ROUTES_SRC, RRS_ROUTES, VSG_VERSION,
VSGXCHANGE_VERSION, VSGIMGUI_VERSION, KTX_VERSION, DOCKER.

Примечание: штатный docker build (без BuildKit) не учитывает значения ARG в
ключе кеша. Если меняете версии зависимостей через переменные окружения,
запускайте с --no-cache.
EOF
}

# --- Разбор аргументов ----------------------------------------------------
while [[ $# -gt 0 ]]; do
  case "$1" in
      -i | --image)        IMAGE="$2"; shift 2 ;;
      -o | --output)       OUTPUT_DIR="$2"; shift 2 ;;
      -t | --build-type)   BUILD_TYPE="$2"; shift 2 ;;
      -j | --jobs)         JOBS="$2"; shift 2 ;;
      --build-only)        DO_BUILD=1; EXPORT=0; shift ;;
      --no-build)          DO_BUILD=0; shift ;;
      --deps-only)         DEPS_ONLY=1; shift ;;
      --no-cache)          NO_CACHE=1; shift ;;
      --pull)              PULL=1; shift ;;
      --routes)            ROUTES="$2"; shift 2 ;;
      --no-routes)         ROUTES="none"; shift ;;
      -h | --help)         usage; exit 0 ;;
      *) echo "Неизвестный параметр: $1" >&2; usage >&2; exit 2 ;;
  esac
done

# --- Проверки -------------------------------------------------------------
if ! command -v "${DOCKER_BIN}" >/dev/null 2>&1; then
    echo "Ошибка: не найден docker (${DOCKER_BIN}). Установите Docker и повторите." >&2
    exit 1
fi

if ! "${DOCKER_BIN}" info >/dev/null 2>&1; then
    echo "Ошибка: демон Docker недоступен (нет прав или не запущен)." >&2
    exit 1
fi

# Имя файла сборки образа; версия Убунту указана в самом имени
DOCKERFILE="${RRS_DOCKERFILE:-${ROOT_DIR}/Dockerfile.ubuntu26.04}"

if [[ ! -f "${DOCKERFILE}" ]]; then
    echo "Ошибка: не найден ${DOCKERFILE}" >&2
    exit 1
fi

if [[ ! "$BUILD_TYPE" =~ ^(Release|RelWithDebInfo|Debug|MinSizeRel)$ ]]; then
    echo "Ошибка: неверный тип сборки '${BUILD_TYPE}'" >&2
    exit 2
fi

if [[ "$DO_BUILD" -eq 0 && "$EXPORT" -eq 0 ]]; then
    echo "Ошибка: указаны --no-build и --build-only одновременно — делать нечего." >&2
    exit 2
fi

# --- Сборка образа --------------------------------------------------------
if [[ "$DO_BUILD" -eq 1 ]]; then
    echo "==> Сборка образа '${IMAGE}' (тип сборки: ${BUILD_TYPE}, задач: ${JOBS})"

    BUILD_ARGS=(
      --file "${DOCKERFILE}"
      --build-arg "BUILD_TYPE=${BUILD_TYPE}"
      --build-arg "JOBS=${JOBS}"
      --build-arg "VSG_REPO=${VSG_REPO}"
      --build-arg "VSG_VERSION=${VSG_VERSION}"
      --build-arg "VSGXCHANGE_REPO=${VSGXCHANGE_REPO}"
      --build-arg "VSGXCHANGE_VERSION=${VSGXCHANGE_VERSION}"
      --build-arg "VSGIMGUI_REPO=${VSGIMGUI_REPO}"
      --build-arg "VSGIMGUI_VERSION=${VSGIMGUI_VERSION}"
      --build-arg "KTX_REPO=${KTX_REPO}"
      --build-arg "KTX_VERSION=${KTX_VERSION}"
      --build-arg "RRS_ARTIFACT_PREFIX=${INSTALL_PREFIX}"
    )

    # В режиме --deps-only собирается только стадия deps, и складывать её
    # в основной тег нельзя: иначе --no-build ниже полез бы за артефактами в
    # образ без собранного RRS. Поэтому используем отдельный тег с суффиксом.
    BUILD_TAG="${IMAGE}"
    if [[ "$DEPS_ONLY" -eq 1 ]]; then
        BUILD_TAG="${IMAGE}-deps"
        BUILD_ARGS+=(--target deps)
    fi

    [[ "$NO_CACHE" -eq 1 ]] && BUILD_ARGS+=(--no-cache)
    [[ "$PULL" -eq 1 ]] && BUILD_ARGS+=(--pull)

    BUILD_ARGS=(--tag "${BUILD_TAG}" "${BUILD_ARGS[@]}")

    "${DOCKER_BIN}" build "${BUILD_ARGS[@]}" "${ROOT_DIR}"
    echo "==> Образ собран: ${BUILD_TAG}"
else
    echo "==> Пропуск сборки образа (--no-build), используется '${IMAGE}'"
    if ! "${DOCKER_BIN}" image inspect "${IMAGE}" >/dev/null 2>&1; then
        echo "Ошибка: образ '${IMAGE}' не найден." >&2
        exit 1
    fi
fi

if [[ "$DEPS_ONLY" -eq 1 ]]; then
    echo
    echo "==> Готово: собрана только стадия зависимостей (${BUILD_TAG})."
    echo "    Полная сборка: ${0}"
    exit 0
fi

# --- Выгрузка артефактов --------------------------------------------------
if [[ "$EXPORT" -eq 1 ]]; then

    mkdir -p "${OUTPUT_DIR}"
    OUTPUT_DIR="$(cd "${OUTPUT_DIR}" && pwd)"

    # Если в каталоге результата остались файлы прошлых запусков, созданные
    # контейнером (root), текущий пользователь не сможет ни перезаписать их,
    # ни сменить им владельца. Исправляем владельца через одноразовый контейнер.
    if find "${OUTPUT_DIR}" ! -user "$(id -u)" -print -quit 2>/dev/null | grep -q .; then
        echo "==> В ${OUTPUT_DIR} есть файлы не текущего пользователя, исправляю владельца"
        "${DOCKER_BIN}" run --rm --volume "${OUTPUT_DIR}:/out" "${IMAGE}" \
          sh -c "chown -R $(id -u):$(id -g) /out" || true
    fi

    echo "==> Копирование артефактов из образа в ${OUTPUT_DIR}"

    # docker create + docker cp + docker rm: контейнер не запускается, копирование
    # делает демон (файлы сразу принадлежат текущему пользователю), а контейнер
    # гарантированно удаляется после завершения.
    CONTAINER_ID="$("${DOCKER_BIN}" create "${IMAGE}")"
    if [[ -z "${CONTAINER_ID}" ]]; then
        echo "Ошибка: не удалось создать контейнер из образа ${IMAGE}." >&2
        exit 1
    fi

    if ! "${DOCKER_BIN}" cp "${CONTAINER_ID}:${INSTALL_PREFIX}/." "${OUTPUT_DIR}/"; then
        "${DOCKER_BIN}" rm -f "${CONTAINER_ID}" >/dev/null 2>&1 || true
        echo "Ошибка: не удалось скопировать артефакты из контейнера." >&2
        exit 1
    fi

    "${DOCKER_BIN}" rm "${CONTAINER_ID}" >/dev/null

    # --- Маршруты -------------------------------------------------------------
    # В образе их нет (сотни мегабайт, для сборки не нужны), поэтому копируем
    # каталоги маршрутов из исходного дерева в <результат>/routes.
    rrs_copy_routes "${ROUTES_SRC}" "${OUTPUT_DIR}/routes" "${ROUTES}"

    echo
    echo "==> Готово. Результат сборки: ${OUTPUT_DIR}"
    echo "    Содержимое: $(ls "${OUTPUT_DIR}" | sort | tr '\n' ' ')"
    echo "    Пример: ${OUTPUT_DIR}/bin/launcher (или иной исполняемый файл из bin)"

else
  echo "==> Выгрузка артефактов пропущена (--build-only)"
fi
