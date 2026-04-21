#!/bin/bash

benchmark_select_latest_stable_ndk_package() {
  awk -F'|' '
    function trim(value) {
      gsub(/^[[:space:]]+|[[:space:]]+$/, "", value)
      return value
    }

    /^[[:space:]]*ndk;[0-9]/ {
      package_id = trim($1)
      revision = trim($2)

      latest = package_id
      if (revision !~ /(^|[[:space:]])rc[0-9]+$/ &&
          revision !~ /(^|[-[:space:]])beta[0-9]+$/) {
        stable = package_id
      }
    }

    END {
      if (stable != "") {
        print stable
      } else {
        print latest
      }
    }
  '
}

benchmark_ndk_is_prerelease() {
  local ndk_dir="$1"
  local properties_file="$ndk_dir/source.properties"

  if [[ ! -f "$properties_file" ]]; then
    return 1
  fi

  if grep -Eiq '^(Pkg\.Revision|Pkg\.ReleaseName).*(beta|rc)' "$properties_file"; then
    return 0
  fi

  return 1
}

benchmark_find_preferred_ndk_root() {
  local sdk_root="$1"
  local latest_any=""
  local latest_stable=""
  local candidate=""

  if [[ -d "$sdk_root/ndk" ]]; then
    while IFS= read -r candidate; do
      if [[ -z "$latest_any" ]]; then
        latest_any="$candidate"
      fi

      if ! benchmark_ndk_is_prerelease "$candidate"; then
        latest_stable="$candidate"
        break
      fi
    done < <(find "$sdk_root/ndk" -maxdepth 1 -mindepth 1 -type d | sort -Vr)
  fi

  if [[ -n "$latest_stable" ]]; then
    echo "$latest_stable"
    return 0
  fi

  if [[ -n "$latest_any" ]]; then
    echo "$latest_any"
    return 0
  fi

  return 1
}
