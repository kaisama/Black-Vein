#ifndef EXPORTED_VULKAN_FUNCTION
#define EXPORTED_VULKAN_FUNCTION(function)
#endif // !EXPORTED_VULKAN_FUNCTION

EXPORTED_VULKAN_FUNCTION(vkGetInstanceProcAddr)

#undef EXPORTED_VULKAN_FUNCTION

#ifndef GLOBAL_LEVEL_VULKAN_FUNCTION
#define GLOBAL_LEVEL_VULKAN_FUNCTION(function)
#endif // !GLOBAL_LEVEL_VULKAN_FUNCTION

#undef GLOBAL_LEVEL_VULKAN_FUNCTION

#ifndef INSTANCE_LEVEL_VULKAN_FUNCTION
#define INSTANCE_LEVEL_VULKAN_FUNCTION(function)
#endif // !INSTANCE_LEVEL_VULKAN_FUNCTION

#undef INSTANCE_LEVEL_VULKAN_FUNCTION

#ifndef INSTANCE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION
#define INSTANCE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION(function, extension)
#endif // !INSTANCE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION

#undef INSTANCE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION

#ifndef DEVICE_LEVEL_VULKAN_FUNCTION
#define DEVICE_LEVEL_VULKAN_FUNCTION(function)
#endif // !DEVICE_LEVEL_VULKAN_FUNCTION

#undef DEVICE_LEVEL_VULKAN_FUNCTION

#ifndef DEVICE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION
#define DEVICE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION(function, extension)
#endif // !DEVICE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION

#undef DEVICE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION
