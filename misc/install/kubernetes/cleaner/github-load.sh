#!/usr/bin/env bash
#
# Downloads + extracts a GitHub repository (any branch/tag/commit) as .tar.gz
# Uses only curl + tar (no git, no unzip required)
#
# Usage examples:
#   ./download-repo.sh owner/repo
#   ./download-repo.sh owner/repo v1.2.3
#   ./download-repo.sh owner/repo main
#   ./download-repo.sh owner/repo develop --output my-project
#

set -euo pipefail

readonly DEFAULT_BRANCH="main"

function show_usage() {
    echo "Usage:"
    echo "    $0 <repository> [ref] [--output DIR]"
    echo
    echo "Examples:"
    echo "    $0 ziglang/zig"
    echo "    $0 ziglang/zig 0.13.0"
    echo "    $0 ziglang/zig master --output zig-src"
    echo
    echo "Arguments:"
    echo "    repository      GitHub repo in format owner/repo"
    echo "    ref             Branch, tag or commit (default: ${DEFAULT_BRANCH})"
    echo "    --output DIR    Extract into this directory (default: repo-name-ref)"
    exit 1
}

# ──────────────────────────────────────────────────────────────────────────────

REPO=""
REF="${DEFAULT_BRANCH}"
OUTPUT_DIR=""

while [[ $# -gt 0 ]]; do                                                      # $# = number of arguments, while there are still arguments to process
    case "$1" in
        --output)                                                             # is $1 = --output then set OUTPUT_DIR to $2 and shift past both
            OUTPUT_DIR="$2"
            shift 2                                                           # shift past --output and the value
            ;;
        --help|-h)                                                            # is $1 = --help or -h then show usage and exit
            show_usage
            ;;
            *)                                                                # default: set REPO or REF
            if [[ -z "${REPO}" ]]; then                                       # is REPO empty (zero length)? if so, set it to $1
                REPO="$1"
            elif [[ -z "${REF}" || "${REF}" == "${DEFAULT_BRANCH}" ]]; then   # is REF empty or DEFAULT_BRANCH? if so, set it to $1
                REF="$1"
            else
                echo "Error: too many positional arguments" >&2               # too many positional arguments, show usage and exit
                show_usage
            fi
            shift
            ;;
    esac                                                                      # end of case (case backwards = esac)
done

if [[ -z "${REPO}" ]]; then                                                   # verify REPO is set
    echo "Error: repository is required (format: owner/repo)" >&2 # redirect to stderr (2)
    show_usage
fi

# Validate repo format
if [[ ! "${REPO}" =~ ^[a-zA-Z0-9][a-zA-Z0-9-]{0,38}/[a-zA-Z0-9._-]+$ ]]; then
    echo "Error: invalid repository format. Use: owner/repo" >&2
    exit 1
fi

OWNER="${REPO%%/*}"    # extract owner from REPO (before /)
REPO_NAME="${REPO#*/}" # extract repo name from REPO (after /)

# If user didn't specify output directory → create meaningful default
if [[ -z "${OUTPUT_DIR}" ]]; then
    # Replace / with - in ref (tags like v1.2.3-rc.1 → v1.2.3-rc.1)
    SAFE_REF="${REF//\//-}"
    OUTPUT_DIR="${REPO_NAME}-${SAFE_REF}"
fi

# ──────────────────────────────────────────────────────────────────────────────

DOWNLOAD_URL="https://github.com/${REPO}/archive/refs/heads/${REF}.tar.gz"

# For tags we should prefer /tags/ path (GitHub redirects, but cleaner)
if [[ "${REF}" =~ ^[0-9a-f]{7,40}$ ]]; then
    # commit SHA → use /archive/ directly
    DOWNLOAD_URL="https://github.com/${REPO}/archive/${REF}.tar.gz"
elif [[ "${REF}" != "${DEFAULT_BRANCH}" && "${REF}" != "main" && "${REF}" != "master" ]]; then
    # most likely a tag
    DOWNLOAD_URL="https://github.com/${REPO}/archive/refs/tags/${REF}.tar.gz"
fi

echo "Repository  : ${REPO}"
echo "Ref         : ${REF}"
echo "Download URL: ${DOWNLOAD_URL}"
echo "Target dir  : ${OUTPUT_DIR}"
echo

# ──────────────────────────────────────────────────────────────────────────────

echo "→ Downloading archive..."

curl --fail --location --progress-bar \
    --user-agent "download-repo.sh (curl)" \
    -o "${REPO_NAME}.tar.gz" \
    "${DOWNLOAD_URL}" || {
        echo
        echo "Error: download failed." >&2
        echo "  URL: ${DOWNLOAD_URL}" >&2
        rm -f "${REPO_NAME}.tar.gz" 2>/dev/null
        exit 1
    }

# ──────────────────────────────────────────────────────────────────────────────

echo "→ Extracting..."

# Create output directory
mkdir -p "${OUTPUT_DIR}"

# Extract and strip first component (owner-repo-ref → contents)
tar --strip-components=1 \
    --directory "${OUTPUT_DIR}" \
    -xzf "${REPO_NAME}.tar.gz" || {
        echo "Error: extraction failed." >&2
        exit 1
    }

# Cleanup
rm -f "${REPO_NAME}.tar.gz"

echo
echo "Done."
echo "Repository extracted to: ${OUTPUT_DIR}/"
ls -la "${OUTPUT_DIR}" | head -n 12
[[ $(ls -A "${OUTPUT_DIR}" | wc -l) -gt 10 ]] && echo "..."
