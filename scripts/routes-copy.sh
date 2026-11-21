#!/usr/bin/env bash
#
# Копирование маршрутов в дерево результата сборки.
#
# Маршруты не входят в состав образа: это сотни мегабайт, которые не нужны
# для сборки. Поэтому скрипты сборки переносят каталоги из routes/ в
# <результат>/routes/<имя> — лаунчер и симулятор ищут маршруты именно там
# (routes/<имя>/description.xml, траектории, сценарии, топология).
#
# Подключение: . "${SCRIPT_DIR}/routes-copy.sh"
#
# Функция:
#   rrs_copy_routes <каталог-источник> <каталог-назначения> <спец>
#     спец: all | пусто  — все каталоги маршрутов;
#           none         — не копировать (ключ --no-routes);
#           иначе        — список имён через запятую (пробелы отбрасываются).

rrs_copy_routes() {
    local src_dir="$1"
    local dst_dir="$2"
    local spec="$3"

    if [[ "${spec}" == "none" ]]; then
        echo "==> Копирование маршрутов пропущено (--no-routes)"
        # Каталог создаём пустым: лаунчер и симулятор ожидают его наличие.
        mkdir -p "${dst_dir}"
        return 0
    fi

    if [[ ! -d "${src_dir}" ]]; then
        echo "ВНИМАНИЕ: каталог маршрутов ${src_dir} не найден, пропускаю." >&2
        return 0
    fi

    local -a names=()
    local name item

    if [[ -z "${spec}" || "${spec}" == "all" ]]; then
        while IFS= read -r name; do
            names+=("${name}")
        done < <(find "${src_dir}" -mindepth 1 -maxdepth 1 -type d -printf '%f\n' | sort)
    else
        local -a items=()
        IFS=',' read -r -a items <<< "${spec}"
        for item in "${items[@]}"; do
            item="${item#"${item%%[![:space:]]*}"}"
            item="${item%"${item##*[![:space:]]}"}"
            [[ -n "${item}" ]] && names+=("${item}")
        done
    fi

    if [[ "${#names[@]}" -eq 0 ]]; then
        echo "ВНИМАНИЕ: в ${src_dir} нет каталогов маршрутов." >&2
        return 0
    fi

    mkdir -p "${dst_dir}"
    for name in "${names[@]}"; do
        if [[ ! -d "${src_dir}/${name}" ]]; then
            echo "ВНИМАНИЕ: маршрут '${name}' не найден в ${src_dir}, пропускаю." >&2
            continue
        fi
        echo "==> Маршрут '${name}' -> ${dst_dir}/${name}"
        cp -a "${src_dir}/${name}" "${dst_dir}/"
    done
}