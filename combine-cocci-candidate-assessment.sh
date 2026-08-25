#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
repo_arg=""

usage() {
  echo "Usage: $0 [--repo <edk2-repo>] [assessment-dir] [output-file]" >&2
  exit 2
}

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

assessment_dir="${1:-cocci-candidate-assessment}"
output_file="${2:-${assessment_dir}/combined.json}"
EDK2_REPO="${EDK2_REPO:-${repo_arg:-${SCRIPT_DIR}/../edk2}}"

if [[ ! -d "${assessment_dir}" ]]; then
  echo "error: assessment directory not found: ${assessment_dir}" >&2
  exit 2
fi

if [[ ! -d "${EDK2_REPO}" ]]; then
  echo "error: edk2 repo not found: ${EDK2_REPO}" >&2
  exit 2
fi

if ! git -C "${EDK2_REPO}" rev-parse --show-toplevel >/dev/null 2>&1; then
  echo "error: not a git repo: ${EDK2_REPO}" >&2
  exit 2
fi

tmp_results="$(mktemp)"
tmp_commits="$(mktemp)"
trap 'rm -f "${tmp_results}" "${tmp_commits}"' EXIT

git -C "${EDK2_REPO}" log --format='%H' > "${tmp_commits}"

while IFS= read -r commit; do
  result_file="${assessment_dir}/${commit}.json"
  if [[ -f "${result_file}" ]]; then
    cat "${result_file}" >> "${tmp_results}"
    printf '\n' >> "${tmp_results}"
  fi
done < "${tmp_commits}"

jq -s \
  --arg branch "$(git -C "${EDK2_REPO}" rev-parse --abbrev-ref HEAD)" \
  --arg criteria_file "cocci-candidate-criteria.md" \
  --arg assessment_dir "${assessment_dir}" \
  '{
    branch: $branch,
    criteria_file: $criteria_file,
    assessment_dir: $assessment_dir,
    total_commits: length,
    suitable_count: ([.[] | select(.suitable == true)] | length),
    results: .
  }' \
  "${tmp_results}" > "${output_file}"
