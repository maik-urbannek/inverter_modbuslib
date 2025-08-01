#!/bin/bash

SOURCE="$(readlink -f ${BASH_SOURCE[0]})"
ROOT="${SOURCE%/*}"
unset SOURCE
CURRENT="$(pwd)"

exit_and_clean_up()
{
    cd "${CURRENT}"
    exit "${1}"
}

print_help_and_leave()
{
    echo "ussage:"
    echo "${0} [--debug|--release] [--gcc|--clang] [--all] [--docu] [--manual] [--test] [--coverage] [--format] [--windows] [--rp2040] [--esp32] [--guionly] [--requestonly] [--install]"
    echo "    '--release' and '--gcc' are defalut."
    exit_and_clean_up "${1}"
}

set_and_check_for_conflict() # program arg valueSame nameSame valueOther nameOther
{
    if [ -z "${!1}" ]; then
        eval "${1}"=${2}
        return 0
    elif [[ "${!1}" == "${2}" ]]; then
        echo "'${3}' more than once."
    elif [[ "${!1}" == "${4}" ]]; then
        echo "Cant use '${3}' and '${5}' together."
    else
        echo "Internal error while parsing arguments"
    fi
    print_help_and_leave 1
}

set_default_if_empty()
{
    if [ -z "${!1}" ]; then
        eval "${1}"=${2}
    fi
}

parse_arguments()
{
    TASK_COMPILE="false"
    TASK_COVARGE="false"
    TASK_TEST="false"
    TASK_DOCU="false"
    TASK_MANUAL="false"
    TASK_FORMAT="false"
    TASK_WINDOWS="false"
    TASK_RP2040="false"
    TASK_ESP32="false"
    TASK_INSTALL="false"
    ARG_GUI_ONLY="-DCMAKE_IGNORE=ingnor"
    ARG_REQUEST_ONLY="-DCMAKE_IGNORE=ingnor"

    if [[ ${#} -eq 0 ]]; then
        TASK_COMPILE="true"
    fi
    while [[ ${#} -gt 0 ]]; do
        key="${1}"
        shift
        case ${key} in
          -h|--help)
            print_help_and_leave 0
            ;;
          --debug)
            set_and_check_for_conflict "BUILD_TYPE" "DEBUG" "--debug" "RELEASE" "--release"
            TASK_COMPILE="true"
            ;;
          --release)
            set_and_check_for_conflict "BUILD_TYPE" "RELEASE" "--release" "DEBUG" "--debug"
            TASK_COMPILE="true"
            ;;
          --gcc)
            set_and_check_for_conflict "COMPILER" "g++" "--gcc" "clang++" "--clang"
            TASK_COMPILE="true"
            ;;
          --clang)
            set_and_check_for_conflict "COMPILER" "clang++" "--clang" "g++" "--gcc"
            TASK_COMPILE="true"
            ;;
          --docu)
            TASK_DOCU="true"
            ;;
          --manual)
            TASK_MANUAL="true"
            ;;
          --format)
            TASK_FORMAT="true"
            ;;
          --all)
            TASK_DOCU="true"
            TASK_COVERAGE="true"
            TASK_TEST="true"
            TASK_COMPILE="true"
            ;;
          --coverage)
            TASK_COVERAGE="true"
            TASK_TEST="true"
            TASK_COMPILE="true"
            ;;
          --test)
            TASK_TEST="true"
            TASK_COMPILE="true"
            ;;
          --windows)
            TASK_WINDOWS="true"
            ;;
          --rp2040)
            TASK_RP2040="true"
            ;;
          --esp32)
            TASK_ESP32="true"
            ;;
          --guionly)
            TASK_COMPILE="true"
            ARG_GUI_ONLY="-DCMAKE_GUI_ONLY=true"
            BUILD_TYPE="RELEASE"
            ;;
          --requestonly)
            TASK_COMPILE="true"
            ARG_REQUEST_ONLY="-DCMAKE_REQUEST_ONLY=true"
            BUILD_TYPE="RELEASE"
            ;;
          --install)
            TASK_INSTALL="true"
            ;;
          *)
            echo "Unknown argument '${key}'"
            print_help_and_leave 1
          ;;
      esac
    done

    if [[ "${TASK_COMPILE}" == "true" ]]; then
        set_default_if_empty "BUILD_TYPE" "RELEASE"
        set_default_if_empty "COMPILER" "g++"
    fi
}

create_link()
{
    if [ ! -L "${2}" ]; then
        ln -sr "${ROOT}/${1}" "${2}"
    fi
}

format()
{
    FORMAT_COMAND="clang-format-14"
    if [[ "${TASK_FORMAT}" == "true" ]]; then
        echo "Format code."
        find "${ROOT}/include" -type f -regex ".*\.\(c\|cpp\|h\|hpp\)" -exec "${FORMAT_COMAND}" -i {} +
        find "${ROOT}/json"    -type f -regex ".*\.\(c\|cpp\|h\|hpp\)" -exec "${FORMAT_COMAND}" -i {} +
        find "${ROOT}/modbus"  -type f -regex ".*\.\(c\|cpp\|h\|hpp\)" -exec "${FORMAT_COMAND}" -i {} +
        find "${ROOT}/random"  -type f -regex ".*\.\(c\|cpp\|h\|hpp\)" -exec "${FORMAT_COMAND}" -i {} +
        find "${ROOT}/string"  -type f -regex ".*\.\(c\|cpp\|h\|hpp\)" -exec "${FORMAT_COMAND}" -i {} +
        find "${ROOT}/test"    -type f -regex ".*\.\(c\|cpp\|h\|hpp\)" -exec "${FORMAT_COMAND}" -i {} +
        find "${ROOT}/example" -type f -regex ".*\.\(c\|cpp\|h\|hpp\)" -exec "${FORMAT_COMAND}" -i {} +
        js-beautify -r "${ROOT}/node-red/node-solarinvert/batteryinvert.js" > /dev/null
    fi
}

get_git_version()
{
    RESULT=$(git describe --dirty --always --long --match 'v[0-9]*.[0-9]*.[0-9]*' 2>&1)
    V=$(echo "${RESULT}"| head -n 1 | cut -b1)
    if [ "${V}" != "v" ]; then
        echo "EMPTY"
    else
        MAJOR=$(echo "${RESULT}" | cut -dv -f2 | cut -d. -f1)
        MINOR=$(echo "${RESULT}" | cut -dv -f2 | cut -d. -f2)
        PATCH=$(echo "${RESULT}" | cut -dv -f2 | cut -d. -f3 | cut -d- -f1)
        COMMIT=$(echo "${RESULT}" | cut -d- -f2 | cut -d- -f1)

        if [ -z $(echo "${RESULT}" | grep "dirty") ]; then
            DIRTY=""
        else
            DIRTY="dirty "
        fi
        HASH=$(git rev-parse HEAD)
        echo "${MAJOR}.${MINOR}.${PATCH}-${COMMIT} ${DIRTY}${HASH}"
    fi
}

add_hook_if_possible()
{
    cd "${ROOT}"
    if [ -d ".git" ]; then
        create_link "config/.hooks/pre-commit"    ".git/hooks/pre-commit"
    else
        echo "not a git repo. Add no hooks."
    fi
}

compile_project()
{
    if [[ "${TASK_COMPILE}" == "true" ]]; then
        cd "${ROOT}"
        mkdir -p "build" && cd "build"
        cmake -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
              -DCMAKE_CXX_COMPILER="${COMPILER}" \
              -DCMAKE_GIT_VERSION="$(get_git_version)" \
              -DCMAKE_COVERAGE="${TASK_COVERAGE}" \
              "${ARG_REQUEST_ONLY}" \
              "${ARG_GUI_ONLY}" \
              ..
        if [[ "${TASK_COVERAGE}" == "true" ]]; then
            make clean
        fi
        make -j $(nproc)
        if [ ${?} -ne 0 ]; then
            echo "build failed."
            exit_and_clean_up 1
        fi
        if [[ "${TASK_INSTALL}" == "true" ]]; then
            sudo make install
        fi
    fi
}

compile_project_cross_windows()
{
    if [[ "${TASK_WINDOWS}" == "true" ]]; then
        cd "${ROOT}"
        mkdir -p "build/mingw" && cd "build/mingw"
        cmake -DCMAKE_TOOLCHAIN_FILE=../../config/mingw.cmake \
              -DCMAKE_BUILD_TYPE="RELEASE" \
              -DCMAKE_GIT_VERSION="$(get_git_version)" \
              -DCMAKE_COVERAGE="${TASK_COVERAGE}" \
              ../..
        make -j $(nproc)
        if [ ${?} -ne 0 ]; then
            echo "build failed."
            exit_and_clean_up 1
        fi
    fi
}

compile_project_cross_rp2040()
{
    if [[ "${TASK_RP2040}" == "true" ]]; then
        cd "${ROOT}"
        mkdir -p "build" && cd "build"
        mkdir -p "rp2040" && cd "rp2040"
        cmake -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
              -DCMAKE_GIT_VERSION="$(get_git_version)" \
              ../../example/rp2040 && \
        make -j $(nproc)
        if [ ${?} -ne 0 ]; then
            echo "build failed."
            exit_and_clean_up 1
        fi
    fi
}

compile_project_cross_esp32()
{
    if [[ "${TASK_ESP32}" == "true" ]]; then
        cd "${ROOT}"
        mkdir -p "build" && cd "build"
        mkdir -p "esp32" && cd "esp32"
        cmake -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
              -DCMAKE_GIT_VERSION="$(get_git_version)" \
              ../../example/esp32 && \
        make -j $(nproc)
        if [ ${?} -ne 0 ]; then
            echo "build failed."
            exit_and_clean_up 1
        fi
    fi
}

create_docu()
{
    if [[ "${TASK_DOCU}" == "true" ]]; then
        cd "${ROOT}"
        mkdir -p "build"
        WARNINGS=$(doxygen docu/Doxyfile 2>&1 >/dev/null)
        if [ -z "${WARNINGS}" ]
        then
            echo "Docu created."
        else
            printf "${WARNINGS}\n"
            exit_and_clean_up 1
        fi
    fi
}

check_result()
{
    if [ ${?} -ne 0 ]; then
        echo "${1} failed."
        exit_and_clean_up 1
    fi
}

create_manual()
{
    if [[ "${TASK_MANUAL}" == "true" ]]; then
        cd "${ROOT}"
        mkdir -p "build/manual"
        cd manual

        pandoc --from=markdown --to=pdf -o ../build/manual/Pi_Anleitung.pdf ./Pi_Anleitung.md
        check_result "Pi_Anleitung manual"

        pandoc --from=markdown --to=pdf -o ../build/manual/DummyInverter.pdf ./DummyInverter.md
        check_result "DummyInverter manual"
    fi
}

execute_tests()
{
    if [[ "${TASK_TEST}" == "true" ]]; then
        cd "${ROOT}"
        ./build/bin/test_binary/runtest.sh
        if [ ${?} -ne 0 ]; then
            echo "test failed."
            exit_and_clean_up 1
        fi
    fi
}

create_coverage()
{
    if [[ "${TASK_COVERAGE}" == "true" ]]; then
        cd "${ROOT}"
        mkdir -p "build/coverage"
        gcovr --html-details  --print-summary -o build/coverage/coverage.html --exclude  test* --root .
        if [ ${?} -ne 0 ]; then
            echo "test failed."
            exit_and_clean_up 1
        fi
    fi
}


add_hook_if_possible
parse_arguments "${@}"
format
compile_project
compile_project_cross_windows
compile_project_cross_rp2040
compile_project_cross_esp32
execute_tests
create_coverage
create_docu
create_manual

exit_and_clean_up 0
