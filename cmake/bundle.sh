#!/bin/bash

set -euo pipefail

destination="${1}"
chrpath_binary="${2}"
input="${3}"

function join { local IFS="$1"; shift; echo "${*}"; }
exclusion=$(join '|' \
  .*libibverbs\.so.* \
  .*libxcb\.so.* \
  .*libSM\.so.* \
  .*libc\.so.* \
  .*libz\.so.* \
  .*libm\.so.* \
  .*librt\.so.* \
  .*libfont\.so.* \
  .*libfreetype\.so.* \
  .*libaudio\.so.* \
  .*libICE\.so.* \
  .*libglapi\.so.* \
  .*libglib\.so.* \
  .*libgobject\.so.* \
  .*libdl\.so.* \
  .*libX.*\.so.* \
  .*libGL\.so.* \
  .*libpthread\.so.* \
  .*libgthread\.so.* \
  .*libreadline\.so.* \
)

rm -rf "${destination}"
mkdir -p "${destination}"

did_not_fail=false
function remove_output_on_error
{
  if ! ${did_not_fail}
  then
    rm -rf "${destination}"
  fi
}
trap remove_output_on_error EXIT

for dependency_and_path in $( LD_BIND_NOW=1 ldd "${input}" \
                            | grep -vE "${exclusion}" \
                            | grep '=> \(not\|/\)' \
                            | awk '{printf("%s:%s\n", $1, $3)}' \
                            )
do
  dependency="${dependency_and_path%:*}"
  path="${dependency_and_path#*:}"

  if [ "${path}" = "not" ]
  then
    echo >&2 "cannot resolve dependency '${dependency}'"
    exit 2
  fi

  dest="$(readlink -f "${destination}/${dependency}")"
  cp "$(readlink -f "${path}")" "${dest}"
  chmod +w "${dest}"

  "${chrpath_binary}" -d "${dest}"
done

did_not_fail=true
