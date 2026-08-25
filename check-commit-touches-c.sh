#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"

usage() {
  echo "Usage: $0 [--repo <edk2-repo>] <commit-ish>" >&2
  exit 2
}

repo_arg=""

while [[ $# -gt 0 ]]; do
  case "$1" in
    --repo)
      if [[ $# -lt 2 ]]; then
        usage
      fi
      repo_arg="$2"
      shift 2
      ;;
    --help|-h)
      usage
      ;;
    --*)
      usage
      ;;
    *)
      break
      ;;
  esac
done

if [[ $# -ne 1 ]]; then
  usage
fi

commit="$1"
EDK2_REPO="${EDK2_REPO:-${repo_arg:-${SCRIPT_DIR}/../edk2}}"

if [[ ! -d "${EDK2_REPO}" ]]; then
  echo "error: edk2 repo not found: ${EDK2_REPO}" >&2
  exit 2
fi

if ! git -C "${EDK2_REPO}" rev-parse --show-toplevel >/dev/null 2>&1; then
  echo "error: not a git repo: ${EDK2_REPO}" >&2
  exit 2
fi

if ! git -C "${EDK2_REPO}" rev-parse --verify "${commit}^{commit}" >/dev/null 2>&1; then
  echo "error: invalid commit '${commit}'" >&2
  exit 2
fi

c_files="$(
  git -C "${EDK2_REPO}" diff-tree --no-commit-id --name-only -r "${commit}" \
    | grep -E '\.(c|h)$' || true
)"

if [[ -n "${c_files}" ]]; then
  echo "touches_c=true"
  printf '%s\n' "${c_files}"
  exit 0
fi

echo "touches_c=false"
exit 1
