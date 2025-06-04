FROM mcr.microsoft.com/devcontainers/cpp:0-ubuntu-22.04

# Install Arduino CLI
RUN curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh

# Confirm arduino-cli is in /usr/local/bin
RUN ls -l /usr/local/bin

RUN arduino-cli core update-index && arduino-cli core install arduino:avr


