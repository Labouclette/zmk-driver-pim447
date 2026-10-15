
# Copyright (c) 2023-2025 The ZMK Contributors
# SPDX-License-Identifier: MIT

# Make the module headers available
zephyr_include_directories(include)

# Compile the PIM447 driver when enabled
if(CONFIG_ZMK_TRACKBALL_PIM447)
  add_subdirectory(src/drivers/sensor/trackball_pim447)
endif()

# Compile the trackball mode behavior when enabled
if(CONFIG_ZMK_BEHAVIOR_TRACKBALL_MODE)
  add_subdirectory(src/behaviors/behavior_trackball_mode)
endif()
