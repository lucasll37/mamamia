# Instalação Local do MIXR

Instalação concluída em: `/home/lima/Project/mamamia/cpp/mixr_dev`

## Estrutura de Diretórios

```
/home/lima/Project/mamamia/cpp/mixr_dev/
├── mixr/
│   ├── src/                 # Código fonte MIXR
│   │   └── makedefs         # Configuração de build (usa MIXR_ROOT e MIXR_3RD_PARTY_ROOT)
│   └── lib/                 # Bibliotecas MIXR compiladas
├── mixr-3rdparty/
│   ├── lib/                 # Bibliotecas 3rd party (.a)
│   └── include/             # Headers 3rd party
├── mixr-3rdpartysrc/        # Código fonte das dependências
├── mixr-examples/           # Exemplos do MIXR
├── downloads/               # Arquivos baixados originais
├── setenv.sh                # Configuração de ambiente (uso interno)
├── setup_mixr_env.sh        # Configuração de ambiente (uso do usuário)
├── reinstall_dependency.sh  # Reinstalar dependência específica
└── build_*.log              # Logs de compilação
```

## Como o MIXR Encontra as Bibliotecas

O arquivo `makedefs` em `mixr/src/` usa duas variáveis de ambiente:

- **MIXR_ROOT**: Aponta para o diretório `mixr/`
- **MIXR_3RD_PARTY_ROOT**: Aponta para `mixr-3rdparty/`

O script `setup_mixr_env.sh` configura essas variáveis automaticamente.

## Uso Diário

**Sempre execute antes de compilar ou executar:**

```bash
source setup_mixr_env.sh
```

## Compilar Exemplos

```bash
source setup_mixr_env.sh
cd mixr-examples/testGraphics1
make
./testGraphics1
```

## Reinstalar Dependência Específica

```bash
./reinstall_dependency.sh jsbsim
```

## Verificar Instalação

```bash
# Ver variáveis de ambiente
source setup_mixr_env.sh

# Ver bibliotecas 3rd party
ls -lh mixr-3rdparty/lib/*.a

# Ver bibliotecas MIXR
ls -lh mixr/lib/*.a

# Ver headers 3rd party
ls mixr-3rdparty/include/
```

## Troubleshooting

### makedefs não encontra as bibliotecas

Certifique-se de ter executado:
```bash
source setup_mixr_env.sh
```

### Verificar se variáveis estão corretas

```bash
echo $MIXR_ROOT
echo $MIXR_3RD_PARTY_ROOT
```

Devem apontar para:
- MIXR_ROOT: `/home/lima/Project/mamamia/cpp/mixr_dev/mixr`
- MIXR_3RD_PARTY_ROOT: `/home/lima/Project/mamamia/cpp/mixr_dev/mixr-3rdparty`
