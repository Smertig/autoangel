# AutoAngel
**autoangel** - open-source C++ library (with [`lua`](https://www.lua.org/) and [`Python`](https://www.python.org/) bindings) that simplifies working with various Angelica3D file formats. Now it supports only `elements.data` (using sELedit config files) and `*.pck` packages.

---

#### Warning - API is unstable, because library is under development.

---

## Dependencies & Requirements
- C++14 compiler (gcc 4.9+, clang 3.6+ or VS 2015+).
- [`Boost`](https://www.boost.org/)
- zlib
- Lua5.3 (optional)
- Python
- CMake 3.1+



## Build
```sh
cd /folder/with/your/projects
git clone --recursive https://github.com/Smertig/autoangel
cd autoangel
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

## Usage
### Python

[`Examples`](https://github.com/Smertig/autoangel/tree/master/examples/py)

Load `elements.data`:
```python
# Import library
from autoangel import *

# Open and load elements.data
edata = elements.data('/path/to/elements.data')
conf = elements.load_cfgs('/path/to/configs/folder')
edata.load(conf)

# Get some info
print('elements.data version is', edata.version)
print([e.Name for e in edata[1]])
```

Output:
```
elements.data version is 12
['Мечи', 'Копья', 'Топоры/Молоты', 'Дальнобойные', 'Каст.', 'Бич', 'Инструменты мистика']
```

Read pck package
```python
from autoangel import *
surf = pck.package('/path/to/surfaces.pck')
surf.load()
for e in surf.find_prefix('surfaces/iconset'):
    print('{} -> {} bytes'.format(e, len(surf.read(e))))
```

Output:
```
surfaces\iconset\iconlist_action.dds -> 32896 bytes
surfaces\iconset\iconlist_action.txt -> 565 bytes
surfaces\iconset\iconlist_faction.dds -> 8320 bytes
surfaces\iconset\iconlist_faction.txt -> 954 bytes
surfaces\iconset\iconlist_guild.dds -> 2097280 bytes
surfaces\iconset\iconlist_guild.txt -> 720 bytes
surfaces\iconset\iconlist_ivtrf.dds -> 16777344 bytes
surfaces\iconset\iconlist_ivtrf.txt -> 173300 bytes
surfaces\iconset\iconlist_ivtrm.dds -> 16777344 bytes
surfaces\iconset\iconlist_ivtrm.txt -> 173400 bytes
surfaces\iconset\iconlist_pet.dds -> 131200 bytes
surfaces\iconset\iconlist_pet.txt -> 1301 bytes
surfaces\iconset\iconlist_skill.dds -> 524416 bytes
surfaces\iconset\iconlist_skill.txt -> 7878 bytes
surfaces\iconset\iconlist_skillgrp.dds -> 32896 bytes
surfaces\iconset\iconlist_skillgrp.txt -> 602 bytes
surfaces\iconset\iconlist_state.dds -> 32896 bytes
surfaces\iconset\iconlist_state.txt -> 1632 bytes
```

### Lua

[`Examples`](https://github.com/Smertig/autoangel/tree/master/examples/lua)

Load `elements.data`:
```lua
-- Import library
require 'autoangel'

-- Open and load elements.data
edata = elements.data('/path/to/elements.data')
conf = elements.load_cfgs('/path/to/configs/folder')
edata:load(conf)

-- Get some info
print(string.format('elements.data version is %d', edata.version))
for _, e in pairs(edata[1]) do
	print(string.format('[%d] %s', e.ID, e.Name))
end
```

Output:
```
elements.data version is 12
[1] Мечи
[5] Копья
[9] Топоры/Молоты
[13] Дальнобойные
[182] Каст.
[291] Бич
[292] Инструменты мистика
```

## TODO
- [ ] Fill TODO list
- [ ] Python API docs
- [ ] lua API docs
- [ ] pck.package for `lua`
- [ ] More examples
- [ ] Dependency installation guide
- [ ] Installation
- [ ] Make separate `autoangel.a` and link it to lua/python implementations