# lora_lite docs

See [../README.md](../README.md) for a project overview. This document collects additional background and API notes.

## Differences from gr-lora_sdr
lora_lite evolved from the [gr-lora_sdr](https://github.com/daniestevez/gr-lora_sdr) project but deliberately diverges in several ways:

- **Standalone C implementation** – all signal-processing blocks are written in portable C without a dependency on GNU Radio, reducing external requirements and easing integration in small environments.
- **Modular block structure** – each component is an independent module with its own tests. This contrasts with gr-lora_sdr’s monolithic GNU Radio flowgraph where blocks are primarily exercised within the full pipeline.
- **Simplified build system** – lora_lite uses only CMake and CTest instead of GNU Radio’s out‑of‑tree module infrastructure, resulting in a quicker setup and easier CI integration.
- **Current limitations** – the focus on self‑contained modules means features such as real‑time streaming and hardware radio drivers, present in gr-lora_sdr, are not yet exposed.

Developers seeking the original GNU Radio out-of-tree module can find it unmodified in [`legacy_gr_lora_sdr/`](../legacy_gr_lora_sdr/).

## Callback-based I/O and Logging APIs
`lora_io.h` exposes a callback structure so applications can route I/O through files, UARTs, or custom transports:

```c
#include "lora_io.h"

lora_io_t io;
FILE *fp = fopen("input.bin", "rb");
lora_io_init_file(&io, fp);
io.read(io.ctx, buffer, sizeof buffer);
```

Logging macros in `lora_log.h` provide lightweight printf‑style diagnostics. Define `LORA_LITE_ENABLE_LOGGING` at compile time to activate them or override `LORA_LOG_PRINTF` to redirect output:

```c
#include "lora_log.h"

LORA_LOG_INFO("Received %u bytes", count);
```

## Credits and Licensing
Portions of the modules originated from the [`gr-lora_sdr`](https://github.com/daniestevez/gr-lora_sdr) project (GPLv3). The project integrates [`liquid-dsp`](https://github.com/jgaeddert/liquid-dsp), which is distributed under the MIT license. See [`LICENSE`](../LICENSE) for the terms governing lora_lite itself.

## Citation
If lora_lite contributes to your research, please cite the work referenced in [`CITATION.cff`](../CITATION.cff).
