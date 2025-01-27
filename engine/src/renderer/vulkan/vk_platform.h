#pragma once

#include "defines.h"
#include "platform/platform.h"

b8 platformCreateVulkanSurface(vulkanContext* context);

/**
 * Appends the names of required extensions for this platform to
 * the names_darray, which should be created and passed in
 */
void platformGetRequiredExtensionNames(const char*** names_darray);