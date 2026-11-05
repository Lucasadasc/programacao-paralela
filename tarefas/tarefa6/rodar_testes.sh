#!/usr/bin/env bash
# Script Shell para compilar e rodar os testes da Tarefa 6 chamando o script python
set -e

cd "$(dirname "$0")"

chmod +x rodar_testes.py
python3 rodar_testes.py
