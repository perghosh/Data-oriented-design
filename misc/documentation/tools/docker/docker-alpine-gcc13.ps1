# build_cpp.ps1

# `$currentDir = $PWD.Path`: Captures the current directory path in a Windows-compatible format.
# `-v "${currentDir}:/app"`: Mounts the current directory (containing ranges_test.cpp) to /app in the container.
# `-w /app`: Sets the working directory to /app where ranges_test.cpp is available.
# `--rm`: Automatically removes the container after execution to avoid name conflicts (replacing --name alpine-cpp20-specific).
# `/bin/sh -c` "...": Runs the commands in the Alpine container’s shell.

$currentDir = $PWD.Path
docker run --name alpine-gcc13 -it -v "${currentDir}:/app" -w /app alpine:3.19 /bin/sh -c "apk update && apk add gcc g++ musl-dev make libstdc++-dev && g++ --version && g++ -o cpp docker-cpp.cpp -std=c++20 && ./cpp && /bin/sh"

# run the script in PowerShell and check the name of ps1 script, if renamed, update the script name accordingly.
# .\docker-alpine-gcc13.ps1

# Copy file into container
# docker cp D:\temp\#install\cleaner\linux\cleaner alpine-gcc13:/app

