# Allow the build system file scanner to follow symlinks.
# Required because vendor/lobo and vendor/projects/* are symlinks
# into lobo-aosp-platform which lives outside the AOSP tree.
export ALLOW_BP_UNDER_SYMLINKS=true
