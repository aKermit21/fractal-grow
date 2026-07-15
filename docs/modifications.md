---
title: "Modifications"
description: "Modifications and adaptations"
---

# Modifications 🔨

To modify and test changed code, it is recommended to use a separate build directory (e.g., `build-dev/`) containing a modified build settings instance:

```shell
meson setup build-dev/
cd build-dev/
meson configure --buildtype=custom --optimization=2
```
This will enable assertions and extensive logging.
