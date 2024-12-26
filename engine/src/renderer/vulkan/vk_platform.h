#pragma once

#include "defines.h"
#include "platform/platform.h"

b8 platformCreateVulkanSurface(platformState *plat_state, vulkanContext* context);

/**
 * Appends the names of required extensions for this platform to
 * the names_darray, which should be created and passed in
 */
void platformGetRequiredExtensionNames(const char*** names_darray);