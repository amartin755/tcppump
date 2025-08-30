#!/bin/bash
set -e

# Optional: Build-Argumente für UID/GID setzen
UID_ARG=$(id -u)
GID_ARG=$(id -g)

# Alle Dockerfile.* im aktuellen Verzeichnis durchsuchen
for dockerfile in Dockerfile.*; do
    # Prüfen, ob Dateien existieren
    [ -e "$dockerfile" ] || continue

    # Tag aus Dateiname ableiten, z.B. Dockerfile.debian -> cpp-env:debian
    tag="tcppump:${dockerfile#Dockerfile.}"

    echo "======================================"
    echo "Building $dockerfile with tag $tag"
    echo "======================================"

    docker build --build-arg UID=$UID_ARG --build-arg GID=$GID_ARG -f "$dockerfile" -t "$tag" .
#    docker build  -f "$dockerfile" -t "$tag" .
done

echo "Alle Dockerfiles erfolgreich gebaut!"
