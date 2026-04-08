# -----------------------------------------------------------------------------
#
# This class generates a JSON file during bitbake build for version tracking.
# Each component that inherits this class will create its own file under /etc.
#
# Usage:
#     inherit mitac-release
#     MITAC_COMPONENT = "bmcweb"
#
# If MITAC_COMPONENT is not specified, the recipe name (PN) will be used as
# the component identifier
#
# -----------------------------------------------------------------------------

DEPENDS += "jq-native"
MITAC_RELEASE_FILE ?= "${D}/etc/${MITAC_COMPONENT:-${PN}}-release.json"

do_install:append() {
    # default COMPONENT is ${PN}
    COMPONENT="${MITAC_COMPONENT:-${PN}}"
    RELEASE_FILE="${MITAC_RELEASE_FILE}"
    JQ="${STAGING_BINDIR_NATIVE}/jq"

    # fetch git commit ID and branch
    if [ -d "${S}/.git" ]; then
        GIT_COMMIT=$(cd ${S} && git rev-parse --short HEAD || echo "unknown")
        GIT_BRANCH=$(cd ${S} && git rev-parse --abbrev-ref HEAD || echo "unknown")
    else
        GIT_COMMIT="unknown"
        GIT_BRANCH="unknown"
    fi

    BUILD_DATE=$(date '+%Y-%m-%d %H:%M:%S')

    install -d $(dirname ${RELEASE_FILE})

    # create an empty JSON first if the file does not exist
    if [ ! -f "${RELEASE_FILE}" ]; then
        echo "{}" > "${RELEASE_FILE}"
    fi

    # update JSON using jq
    TMPFILE=$(mktemp)
    ${JQ} --arg comp "$COMPONENT" \
          --arg commit "$GIT_COMMIT" \
          --arg branch "$GIT_BRANCH" \
          --arg date "$BUILD_DATE" \
          '.[$comp] = {"Git Commit": $commit, "Branch": $branch, "Build Date": $date}' \
          "${RELEASE_FILE}" > "${TMPFILE}" && mv "${TMPFILE}" "${RELEASE_FILE}"

    chmod 0644 "${RELEASE_FILE}"
}

FILES:${PN} += "/etc/${MITAC_COMPONENT:-${PN}}-release.json"

