#include <Windows.h>
#include <stdint.h>
#include <vulkan.h>
#include <iostream>
#include <vector>
#include <CinderMath.h>
#include <SPIRV\GlslangToSpv.h>

using namespace Cinder::Math;

struct VK_Func
{
	PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr; 
	PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr;
	PFN_vkCreateInstance vkCreateInstance; 
	PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices;  
	PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties;  
	PFN_vkCreateDevice vkCreateDevice;  
	PFN_vkCreateCommandPool vkCreateCommandPool; 
	PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers; 
	PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR;
	PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR;
	PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR;
	PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR;
	PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR;
	PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR;
	PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR;
	PFN_vkCreateImageView vkCreateImageView;
	PFN_vkCreateImage vkCreateImage;
	PFN_vkGetPhysicalDeviceFormatProperties vkGetPhysicalDeviceFormatProperties;
	PFN_vkGetImageMemoryRequirements vkGetImageMemoryRequirements;
	PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties;
	PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties;
	PFN_vkAllocateMemory vkAllocateMemory;
	PFN_vkBindImageMemory vkBindImageMemory;
	PFN_vkCreateBuffer vkCreateBuffer;
	PFN_vkGetBufferMemoryRequirements vkGetBufferMemoryRequirements;
	PFN_vkMapMemory vkMapMemory;
	PFN_vkUnmapMemory vkUnmapMemory;
	PFN_vkBindBufferMemory vkBindBufferMemory;
	PFN_vkCreateDescriptorSetLayout vkCreateDescriptorSetLayout;
	PFN_vkCreatePipelineLayout vkCreatePipelineLayout;
	PFN_vkCreateDescriptorPool vkCreateDescriptorPool;
	PFN_vkAllocateDescriptorSets vkAllocateDescriptorSets;
	PFN_vkUpdateDescriptorSets vkUpdateDescriptorSets;
	PFN_vkCreateSemaphore vkCreateSemaphore;
	PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR;
	PFN_vkCreateRenderPass vkCreateRenderPass;
	PFN_vkCreateShaderModule vkCreateShaderModule;

	PFN_vkDestroyDevice vkDestroyDevice;
	PFN_vkGetDeviceQueue vkGetDeviceQueue;
	PFN_vkQueueWaitIdle vkQueueWaitIdle;
};

struct SwapChainBuffer 
{
	VkImage Image;
	VkImageView View;
};

struct DepthBuffer 
{
	VkFormat Format;
	VkImage Image;
	VkImageView View;
	VkDeviceMemory Memory;
};

struct BufferData 
{
	VkBuffer Buffer;
	VkDeviceMemory Memory;
	VkDescriptorBufferInfo BufferInfo;
};

struct TransformMatrices
{
	mat4f MVP;
	mat4f ModelMat;
	mat4f ViewMat;
	mat4f ProjectionMat;
	mat4f CorrectionMat;
};

struct VK_Data
{
	VkInstance Instance;	
	std::vector<VkPhysicalDevice> PhysicalDevices;
	std::vector<VkPhysicalDeviceProperties> PhysicalDevicesProperties;
	VkPhysicalDeviceMemoryProperties MemoryProperties;

	uint32_t QueueFamilyPropertiesCount;
	uint32_t GraphicsQueueFamilyIndex; 
	uint32_t PresentQueueFamilyIndex; 
	std::vector<VkQueueFamilyProperties> QueueFamilyProperties; 

	VkDevice Device; 

	VkCommandPool CommandPool; 
	VkCommandBuffer CommandBuffer; 

	std::vector<const char*> InstanceExtensionNames; 
	std::vector<const char*> DeviceExtensionNames;

	HWND Window;

	VkSurfaceKHR Surface;
	VkFormat Format;

	uint32_t Width;
	uint32_t Height;

	VkSwapchainKHR SwapChain;
	uint32_t SwapChainImageCount;
	std::vector<SwapChainBuffer> Buffers;

	DepthBuffer Depth;

	TransformMatrices Transform;

	BufferData UniformBuffer;

	std::vector<VkDescriptorSetLayout> DescriptorSetLayouts;

	VkPipelineLayout PipelineLayout;

	VkDescriptorPool DescriptorPool;
	std::vector<VkDescriptorSet> DescriptorSets;

	uint32_t CurrentBuffer;

	VkRenderPass RenderPass;
	VkPipelineShaderStageCreateInfo ShaderStages[2];
};

static VK_Func Vulkan_Functions;
static VK_Data Vulkan_Data; 

void ExitOnError(const char *msg) 
{
	MessageBox(NULL, msg, NULL, MB_ICONERROR);
	std::cout << msg;

	exit(EXIT_FAILURE);
} 

/// NOTE(KAI): Taken form vulkan samples (util.cpp) 
///*NOTE(KAI)*///
/// 1- Goes through each memory type in the physical device memory properties
/// 2- Check which of the type bits is avaiable by &ing it with 1, if not then shift the bits to the right by one (???????!!) :/
/// 3- if the type is available check if it matches the required properties
/// 4- if found return its index
bool GetMemoryTypeFromProperties(VK_Data &data, uint32_t typeBits, VkFlags requirements_mask, uint32_t *typeIndex) 
{
	// Search memtypes to find first index with those properties
	for (uint32_t i = 0; i < data.MemoryProperties.memoryTypeCount; i++) 
	{
		if ((typeBits & 1) == 1) 
		{
			// Type is available, does it match user properties?
			if ((data.MemoryProperties.memoryTypes[i].propertyFlags & requirements_mask) == requirements_mask) 
			{
				*typeIndex = i;
				return true;
			}
		}

		typeBits >>= 1;
	}

	// No memory types matched, return failure
	return false;
}

///***NOTE(KAI)***///
/// Shader utility functions taken from the vulkan samples 
void init_resources(TBuiltInResource &Resources) 
{
	Resources.maxLights = 32;
	Resources.maxClipPlanes = 6;
	Resources.maxTextureUnits = 32;
	Resources.maxTextureCoords = 32;
	Resources.maxVertexAttribs = 64;
	Resources.maxVertexUniformComponents = 4096;
	Resources.maxVaryingFloats = 64;
	Resources.maxVertexTextureImageUnits = 32;
	Resources.maxCombinedTextureImageUnits = 80;
	Resources.maxTextureImageUnits = 32;
	Resources.maxFragmentUniformComponents = 4096;
	Resources.maxDrawBuffers = 32;
	Resources.maxVertexUniformVectors = 128;
	Resources.maxVaryingVectors = 8;
	Resources.maxFragmentUniformVectors = 16;
	Resources.maxVertexOutputVectors = 16;
	Resources.maxFragmentInputVectors = 15;
	Resources.minProgramTexelOffset = -8;
	Resources.maxProgramTexelOffset = 7;
	Resources.maxClipDistances = 8;
	Resources.maxComputeWorkGroupCountX = 65535;
	Resources.maxComputeWorkGroupCountY = 65535;
	Resources.maxComputeWorkGroupCountZ = 65535;
	Resources.maxComputeWorkGroupSizeX = 1024;
	Resources.maxComputeWorkGroupSizeY = 1024;
	Resources.maxComputeWorkGroupSizeZ = 64;
	Resources.maxComputeUniformComponents = 1024;
	Resources.maxComputeTextureImageUnits = 16;
	Resources.maxComputeImageUniforms = 8;
	Resources.maxComputeAtomicCounters = 8;
	Resources.maxComputeAtomicCounterBuffers = 1;
	Resources.maxVaryingComponents = 60;
	Resources.maxVertexOutputComponents = 64;
	Resources.maxGeometryInputComponents = 64;
	Resources.maxGeometryOutputComponents = 128;
	Resources.maxFragmentInputComponents = 128;
	Resources.maxImageUnits = 8;
	Resources.maxCombinedImageUnitsAndFragmentOutputs = 8;
	Resources.maxCombinedShaderOutputResources = 8;
	Resources.maxImageSamples = 0;
	Resources.maxVertexImageUniforms = 0;
	Resources.maxTessControlImageUniforms = 0;
	Resources.maxTessEvaluationImageUniforms = 0;
	Resources.maxGeometryImageUniforms = 0;
	Resources.maxFragmentImageUniforms = 8;
	Resources.maxCombinedImageUniforms = 8;
	Resources.maxGeometryTextureImageUnits = 16;
	Resources.maxGeometryOutputVertices = 256;
	Resources.maxGeometryTotalOutputComponents = 1024;
	Resources.maxGeometryUniformComponents = 1024;
	Resources.maxGeometryVaryingComponents = 64;
	Resources.maxTessControlInputComponents = 128;
	Resources.maxTessControlOutputComponents = 128;
	Resources.maxTessControlTextureImageUnits = 16;
	Resources.maxTessControlUniformComponents = 1024;
	Resources.maxTessControlTotalOutputComponents = 4096;
	Resources.maxTessEvaluationInputComponents = 128;
	Resources.maxTessEvaluationOutputComponents = 128;
	Resources.maxTessEvaluationTextureImageUnits = 16;
	Resources.maxTessEvaluationUniformComponents = 1024;
	Resources.maxTessPatchComponents = 120;
	Resources.maxPatchVertices = 32;
	Resources.maxTessGenLevel = 64;
	Resources.maxViewports = 16;
	Resources.maxVertexAtomicCounters = 0;
	Resources.maxTessControlAtomicCounters = 0;
	Resources.maxTessEvaluationAtomicCounters = 0;
	Resources.maxGeometryAtomicCounters = 0;
	Resources.maxFragmentAtomicCounters = 8;
	Resources.maxCombinedAtomicCounters = 8;
	Resources.maxAtomicCounterBindings = 1;
	Resources.maxVertexAtomicCounterBuffers = 0;
	Resources.maxTessControlAtomicCounterBuffers = 0;
	Resources.maxTessEvaluationAtomicCounterBuffers = 0;
	Resources.maxGeometryAtomicCounterBuffers = 0;
	Resources.maxFragmentAtomicCounterBuffers = 1;
	Resources.maxCombinedAtomicCounterBuffers = 1;
	Resources.maxAtomicCounterBufferSize = 16384;
	Resources.maxTransformFeedbackBuffers = 4;
	Resources.maxTransformFeedbackInterleavedComponents = 64;
	Resources.maxCullDistances = 8;
	Resources.maxCombinedClipAndCullDistances = 8;
	Resources.maxSamples = 4;
	Resources.limits.nonInductiveForLoops = 1;
	Resources.limits.whileLoops = 1;
	Resources.limits.doWhileLoops = 1;
	Resources.limits.generalUniformIndexing = 1;
	Resources.limits.generalAttributeMatrixVectorIndexing = 1;
	Resources.limits.generalVaryingIndexing = 1;
	Resources.limits.generalSamplerIndexing = 1;
	Resources.limits.generalVariableIndexing = 1;
	Resources.limits.generalConstantMatrixVectorIndexing = 1;
}

EShLanguage FindLanguage(const VkShaderStageFlagBits shader_type) 
{
	switch (shader_type) {
	case VK_SHADER_STAGE_VERTEX_BIT:
		return EShLangVertex;

	case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
		return EShLangTessControl;

	case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
		return EShLangTessEvaluation;

	case VK_SHADER_STAGE_GEOMETRY_BIT:
		return EShLangGeometry;

	case VK_SHADER_STAGE_FRAGMENT_BIT:
		return EShLangFragment;

	case VK_SHADER_STAGE_COMPUTE_BIT:
		return EShLangCompute;

	default:
		return EShLangVertex;
	}
}

bool GLSLtoSPV(const VkShaderStageFlagBits shader_type, const char *pshader, std::vector<unsigned int> &spirv) 
{
	EShLanguage stage = FindLanguage(shader_type);
	glslang::TShader shader(stage);
	glslang::TProgram program;
	const char *shaderStrings[1];
	TBuiltInResource Resources;
	init_resources(Resources);

	// Enable SPIR-V and Vulkan rules when parsing GLSL
	EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);

	shaderStrings[0] = pshader;
	shader.setStrings(shaderStrings, 1);

	if (!shader.parse(&Resources, 100, false, messages)) {
		puts(shader.getInfoLog());
		puts(shader.getInfoDebugLog());
		return false;  // something didn't work
	}

	program.addShader(&shader);

	//
	// Program-level processing...
	//

	if (!program.link(messages)) {
		puts(shader.getInfoLog());
		puts(shader.getInfoDebugLog());
		fflush(stdout);
		return false;
	}

	glslang::GlslangToSpv(*program.getIntermediate(stage), spirv);

	return true;
}
///
///

PFN_vkVoidFunction GetFunctionPointer(VkInstance instance, char *name)	
{
	PFN_vkVoidFunction result = 0;

	result = Vulkan_Functions.vkGetInstanceProcAddr(instance, name);

	return result;
}

PFN_vkVoidFunction GetDeviceRelatedFunctionPointer(VkDevice device, char *name) 
{
	PFN_vkVoidFunction result = 0;

	result = Vulkan_Functions.vkGetDeviceProcAddr(device, name);

	return result;
}

///*NOTE(KAI)*///
/// Create a vulkan instance needs:
/// 1- Filling-out the Application info struct
/// 2- Filling-out the Instance info struct
/// 3- Filling-out the Instance info struct may need using extension name or layer names
/// 4- Till now we just used extension names for Surface Extensions
VkResult CreateVulkanInstance() 
{
	VkResult result;

	VkInstanceCreateInfo instanceInfo = {};

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Vulkan app test";
	appInfo.pEngineName = "Vulkan app test";
	appInfo.apiVersion = VK_API_VERSION_1_0;	

	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.pApplicationInfo = &appInfo;

	Vulkan_Data.InstanceExtensionNames.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
	Vulkan_Data.InstanceExtensionNames.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
	
	instanceInfo.enabledExtensionCount = Vulkan_Data.InstanceExtensionNames.size();
	instanceInfo.ppEnabledExtensionNames = Vulkan_Data.InstanceExtensionNames.data();

	result = Vulkan_Functions.vkCreateInstance(&instanceInfo, NULL, &Vulkan_Data.Instance);

	return result;
}

///*NOTE(KAI)*///
/// 1- Get physical devices
/// 2- Get queue family(s)
/// 3- Add device extensions for surfaces (or any other extension we may want)
/// 3- Get Logical device
VkResult CreateVulkanDevice() 
{
	VkResult result;

	// NOTE(KAI): Getting a list of physical devices
	uint32_t deviceCount;

	result = Vulkan_Functions.vkEnumeratePhysicalDevices(Vulkan_Data.Instance, &deviceCount, NULL);

	if (result != VK_SUCCESS)
	{
		ExitOnError("Couldn't get the number of physical devices\n");

		return result;
	}

	Vulkan_Data.PhysicalDevices.resize(deviceCount);

	result = Vulkan_Functions.vkEnumeratePhysicalDevices(Vulkan_Data.Instance
														, &deviceCount
														, Vulkan_Data.PhysicalDevices.data());
	
	if (result != VK_SUCCESS)
	{
		ExitOnError("Couldn't get a list of physical devices\n");

		return result;
	}

	//

	VkDeviceQueueCreateInfo queueInfo = {};

	// NOTE(KAI): Getting a list of queue family properties
	Vulkan_Functions.vkGetPhysicalDeviceQueueFamilyProperties(Vulkan_Data.PhysicalDevices[0]
															, &Vulkan_Data.QueueFamilyPropertiesCount
															, NULL);

	Vulkan_Data.QueueFamilyProperties.resize(Vulkan_Data.QueueFamilyPropertiesCount);

	Vulkan_Functions.vkGetPhysicalDeviceQueueFamilyProperties(Vulkan_Data.PhysicalDevices[0]
															, &Vulkan_Data.QueueFamilyPropertiesCount
															, Vulkan_Data.QueueFamilyProperties.data());

	// NOTE(KAI): Getting the queue family for the graphics operations
	for (size_t i = 0; i < Vulkan_Data.QueueFamilyProperties.size(); i++)
	{
		if (Vulkan_Data.QueueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			queueInfo.queueFamilyIndex = i;
			Vulkan_Data.GraphicsQueueFamilyIndex = i;
			break;
		}
	}

	float queuePriorities[1] = { 1.0 };
	queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueInfo.pQueuePriorities = queuePriorities;
	queueInfo.queueCount = 1;

	VkDeviceCreateInfo deviceInfo = {};
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.queueCreateInfoCount = 1;
	deviceInfo.pQueueCreateInfos = &queueInfo;

	Vulkan_Data.DeviceExtensionNames.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

	deviceInfo.enabledExtensionCount = Vulkan_Data.DeviceExtensionNames.size();
	deviceInfo.ppEnabledExtensionNames = Vulkan_Data.DeviceExtensionNames.data();

	Vulkan_Data.PhysicalDevicesProperties.resize(Vulkan_Data.PhysicalDevices.size());

	for (size_t i = 0; i < Vulkan_Data.PhysicalDevices.size(); i++)
	{
		Vulkan_Functions.vkGetPhysicalDeviceProperties(Vulkan_Data.PhysicalDevices[i], &Vulkan_Data.PhysicalDevicesProperties[i]);
	}

	Vulkan_Functions.vkGetPhysicalDeviceMemoryProperties(Vulkan_Data.PhysicalDevices[0], &Vulkan_Data.MemoryProperties);

	Vulkan_Functions.vkCreateDevice(Vulkan_Data.PhysicalDevices[0], &deviceInfo, NULL, &Vulkan_Data.Device);

	return result;
}

VkResult InitCommandBufferPool() 
{
	VkCommandPoolCreateInfo poolInfo = {};
	
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = Vulkan_Data.GraphicsQueueFamilyIndex;

	VkResult result = Vulkan_Functions.vkCreateCommandPool(Vulkan_Data.Device, &poolInfo, NULL, &Vulkan_Data.CommandPool);

	return result;
}

VkResult InitCommandBuffer() 
{
	VkCommandBufferAllocateInfo bufferInfo = {};
	
	bufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	bufferInfo.commandPool = Vulkan_Data.CommandPool;
	bufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	bufferInfo.commandBufferCount = 1;

	VkResult result = Vulkan_Functions.vkAllocateCommandBuffers(Vulkan_Data.Device, &bufferInfo, &Vulkan_Data.CommandBuffer);

	return result;
}

VkResult InitSurface() 
{
	VkWin32SurfaceCreateInfoKHR surfaceInfo = {};

	surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceInfo.hwnd = Vulkan_Data.Window;

	VkResult result = Vulkan_Functions.vkCreateWin32SurfaceKHR(Vulkan_Data.Instance, &surfaceInfo, NULL, &Vulkan_Data.Surface);

	return result;
}

///*NOTE(KAI)*///
/// To create the swapchain we need:
///		1- Surface
///		2- Graphics queue index
///		3- Present queue index
///		4- Pixel format
///		5- Fill out swapchain info structure:
///			=> Surface capabilities
///			=> Present modes
///			=> Swapchain extents
///			=> Minimum number of images in swapchain
///			=> PreTransform ????
///			=> Composite Alpha flags
///		6- Create Swapchain
///		7- Save Image and image views to use later on
VkResult InitSwapChain()
{
	VkResult res;

	///*NOTE(KAI)*///
	/// Get the graphics and present queue family index
	/// check if there is a queue that supports both present and graphics, if not look for separate queues
	{
		VkBool32 *supportPresent = (VkBool32*)malloc(Vulkan_Data.QueueFamilyPropertiesCount * sizeof(VkBool32));

		for (size_t i = 0; i < Vulkan_Data.QueueFamilyPropertiesCount; i++)
		{
			Vulkan_Functions.vkGetPhysicalDeviceSurfaceSupportKHR(Vulkan_Data.PhysicalDevices[0], i, Vulkan_Data.Surface, &supportPresent[i]);
		}

		Vulkan_Data.GraphicsQueueFamilyIndex = UINT32_MAX;
		Vulkan_Data.PresentQueueFamilyIndex = UINT32_MAX;

		for (size_t i = 0; i < Vulkan_Data.QueueFamilyPropertiesCount; i++)
		{
			if ((Vulkan_Data.QueueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
			{
				if (Vulkan_Data.GraphicsQueueFamilyIndex == UINT32_MAX)
				{
					Vulkan_Data.GraphicsQueueFamilyIndex = i;
				}

				if (supportPresent[i] == VK_TRUE)
				{
					Vulkan_Data.GraphicsQueueFamilyIndex = i;
					Vulkan_Data.PresentQueueFamilyIndex = i;

					break;
				}
			}
		}

		if (Vulkan_Data.PresentQueueFamilyIndex == UINT32_MAX)
		{
			for (size_t i = 0; i < Vulkan_Data.QueueFamilyPropertiesCount; i++)
			{
				if (supportPresent[i] == VK_TRUE)
				{
					Vulkan_Data.PresentQueueFamilyIndex = i;

					break;
				}
			}
		}

		free(supportPresent);

		if (Vulkan_Data.GraphicsQueueFamilyIndex == UINT32_MAX || Vulkan_Data.PresentQueueFamilyIndex == UINT32_MAX)
		{
			ExitOnError("Couldn't find either a suitable graphics queue or present queue or a queue with both capabilities\n");

			return VkResult::VK_ERROR_VALIDATION_FAILED_EXT;
		}
	}

	///*NOTE(KAI)*///
	/// Get supported pixel formats
	/// choose one of them
	{
		uint32_t formatCount;
		res = Vulkan_Functions.vkGetPhysicalDeviceSurfaceFormatsKHR(Vulkan_Data.PhysicalDevices[0]
			, Vulkan_Data.Surface
			, &formatCount
			, NULL);

		if (res != VK_SUCCESS)
		{
			ExitOnError("Failed to get format count\n");

			return VkResult::VK_ERROR_FORMAT_NOT_SUPPORTED;
		}

		VkSurfaceFormatKHR *formats = (VkSurfaceFormatKHR*)malloc(formatCount * sizeof(VkSurfaceFormatKHR));
		res = Vulkan_Functions.vkGetPhysicalDeviceSurfaceFormatsKHR(Vulkan_Data.PhysicalDevices[0]
			, Vulkan_Data.Surface
			, &formatCount
			, formats);

		if (res != VK_SUCCESS)
		{
			ExitOnError("Failed to get format count\n");

			return VkResult::VK_ERROR_FORMAT_NOT_SUPPORTED;
		}
		else if (formatCount < 1)
		{
			ExitOnError("Failed to get format count\n");

			return VkResult::VK_ERROR_FORMAT_NOT_SUPPORTED;
		}

		if (formatCount == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
		{
			Vulkan_Data.Format = VK_FORMAT_B8G8R8A8_UNORM;
		}
		else
		{
			Vulkan_Data.Format = formats[0].format;
		}

		free(formats);
	}
	
	///*NOTE(KAI)*///
	/// Get the data neccessary for the swapchain info struct
	/// Create the swapchain
	/// Get the images and image views
	{
		VkSurfaceCapabilitiesKHR surfaceCapabilities;
		res = Vulkan_Functions.vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
			Vulkan_Data.PhysicalDevices[0]
			, Vulkan_Data.Surface
			, &surfaceCapabilities);

		if (res != VK_SUCCESS)
		{
			ExitOnError("Failed to get surface capabilities\n");

			return VkResult::VK_INCOMPLETE;
		}

		///*NOTE(KAI)*///
		/// Get supported present modes
		/// present modes are how we get images from the swapchain
		/// for now we will choose the default one "VK_PRESENT_MODE_FIFO_KHR"
		/*
		uint32_t presentModeCount;
		res = Vulkan_Functions.vkGetPhysicalDeviceSurfacePresentModesKHR(
			Vulkan_Data.PhysicalDevices[0]
			, Vulkan_Data.Surface
			, &presentModeCount
			, NULL);

		if (res != VK_SUCCESS)
		{
			ExitOnError("Failed to get present modes count\n");

			return VkResult::VK_INCOMPLETE;
		}

		VkPresentModeKHR *presentModes = (VkPresentModeKHR*)malloc(presentModeCount * sizeof(VkPresentModeKHR));
		res = Vulkan_Functions.vkGetPhysicalDeviceSurfacePresentModesKHR(
		Vulkan_Data.PhysicalDevices[0]
		, Vulkan_Data.Surface
		, &presentModeCount
		, presentModes);

		if (res != VK_SUCCESS)
		{
		ExitOnError("Failed to get present modes\n");

		return VkResult::VK_INCOMPLETE;
		}
		*/

		VkExtent2D swapchainExtent;

		if (surfaceCapabilities.currentExtent.width == 0xFFFFFFFF)
		{
			swapchainExtent.width = Vulkan_Data.Width;
			swapchainExtent.height = Vulkan_Data.Height;

			if (swapchainExtent.width < surfaceCapabilities.currentExtent.width)
			{
				swapchainExtent.width = surfaceCapabilities.currentExtent.width;
			}
			else if (swapchainExtent.width > surfaceCapabilities.currentExtent.width)
			{
				swapchainExtent.width = surfaceCapabilities.currentExtent.width;
			}

			if (swapchainExtent.height < surfaceCapabilities.currentExtent.height)
			{
				swapchainExtent.height = surfaceCapabilities.currentExtent.height;
			}
			else if (swapchainExtent.height > surfaceCapabilities.currentExtent.height)
			{
				swapchainExtent.height = surfaceCapabilities.currentExtent.height;
			}
		}
		else
		{
			swapchainExtent = surfaceCapabilities.currentExtent;
		}

		VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;

		uint32_t desiredNumberOfSwapChainImages = surfaceCapabilities.minImageCount;

		VkSurfaceTransformFlagBitsKHR preTransform;

		if (surfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
		{
			preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		}
		else
		{
			preTransform = surfaceCapabilities.currentTransform;
		}

		VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		VkCompositeAlphaFlagBitsKHR compositeAlphaFlags[4] =
		{
			VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
			VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
			VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR
		};

		for (size_t i = 0; i < sizeof(compositeAlphaFlags); i++)
		{
			if (surfaceCapabilities.supportedCompositeAlpha & compositeAlphaFlags[i])
			{
				compositeAlpha = compositeAlphaFlags[i];

				break;
			}
		}

		VkSwapchainCreateInfoKHR swapInfo = {};

		swapInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapInfo.surface = Vulkan_Data.Surface;
		swapInfo.minImageCount = desiredNumberOfSwapChainImages;
		swapInfo.imageFormat = Vulkan_Data.Format;
		swapInfo.imageExtent.width = swapchainExtent.width;
		swapInfo.imageExtent.height = swapchainExtent.height;
		swapInfo.preTransform = preTransform;
		swapInfo.compositeAlpha = compositeAlpha;
		swapInfo.imageArrayLayers = 1;
		swapInfo.presentMode = swapchainPresentMode;
		swapInfo.oldSwapchain = VK_NULL_HANDLE;
		swapInfo.clipped = true;
		swapInfo.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
		swapInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapInfo.queueFamilyIndexCount = 0;
		swapInfo.pQueueFamilyIndices = NULL;

		uint32_t queueFamilyIndices[2] =
		{
			Vulkan_Data.GraphicsQueueFamilyIndex,
			Vulkan_Data.PresentQueueFamilyIndex
		};

		if (queueFamilyIndices[0] != queueFamilyIndices[1])
		{
			swapInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			swapInfo.queueFamilyIndexCount = 2;
			swapInfo.pQueueFamilyIndices = queueFamilyIndices;
		}

		res = Vulkan_Functions.vkCreateSwapchainKHR(Vulkan_Data.Device, &swapInfo, NULL, &Vulkan_Data.SwapChain);

		if (res != VK_SUCCESS)
		{
			ExitOnError("Failed to create swapchain\n");

			return VkResult::VK_INCOMPLETE;
		}


		res = Vulkan_Functions.vkGetSwapchainImagesKHR(Vulkan_Data.Device, Vulkan_Data.SwapChain, &Vulkan_Data.SwapChainImageCount, NULL);

		if (res != VK_SUCCESS)
		{
			ExitOnError("Failed to get swapchain image count\n");

			return VkResult::VK_INCOMPLETE;
		}

		VkImage *images = (VkImage*)malloc(Vulkan_Data.SwapChainImageCount * sizeof(VkImage));

		res = Vulkan_Functions.vkGetSwapchainImagesKHR(Vulkan_Data.Device, Vulkan_Data.SwapChain, &Vulkan_Data.SwapChainImageCount, images);

		if (res != VK_SUCCESS)
		{
			ExitOnError("Failed to get swapchain images\n");

			return VkResult::VK_INCOMPLETE;
		}

		Vulkan_Data.Buffers.resize(Vulkan_Data.SwapChainImageCount);

		for (size_t i = 0; i < Vulkan_Data.SwapChainImageCount; i++)
		{
			Vulkan_Data.Buffers[i].Image = images[i];
		}

		free(images);

		for (size_t i = 0; i < Vulkan_Data.SwapChainImageCount; i++)
		{
			VkImageViewCreateInfo imageInfo = {};
			imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			imageInfo.image = Vulkan_Data.Buffers[i].Image;
			imageInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			imageInfo.format = Vulkan_Data.Format;
			imageInfo.components.r = VK_COMPONENT_SWIZZLE_R;
			imageInfo.components.g = VK_COMPONENT_SWIZZLE_G;
			imageInfo.components.b = VK_COMPONENT_SWIZZLE_B;
			imageInfo.components.a = VK_COMPONENT_SWIZZLE_A;
			imageInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageInfo.subresourceRange.baseMipLevel = 0;
			imageInfo.subresourceRange.levelCount = 1;
			imageInfo.subresourceRange.baseArrayLayer = 0;
			imageInfo.subresourceRange.layerCount = 1;

			res = Vulkan_Functions.vkCreateImageView(Vulkan_Data.Device, &imageInfo, NULL, &Vulkan_Data.Buffers[i].View);

			if (res != VK_SUCCESS)
			{
				ExitOnError("Failed to get swapchain view\n");

				return VkResult::VK_INCOMPLETE;
			}
		}
	}	

	return res;
}

///*NOTE(KAI)*///
/// To create a depth buffer we need:
/// 1- image
/// 2- memory and bind it to the image
/// 3- image view
VkResult InitDepthBuffer() 
{
	VkResult res;

	VkFormat depthFormat = VK_FORMAT_D16_UNORM;
	Vulkan_Data.Depth.Format = depthFormat;

	VkImageCreateInfo imageInfo = {};
	
	VkFormatProperties formatProps;
	Vulkan_Functions.vkGetPhysicalDeviceFormatProperties(Vulkan_Data.PhysicalDevices[0], depthFormat, &formatProps);
	
	if (formatProps.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
	{
		imageInfo.tiling = VK_IMAGE_TILING_LINEAR;
	}
	else if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
	{
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	}
	else
	{
		ExitOnError("D16 depth format not supported\n");

		return VkResult::VK_INCOMPLETE;
	}

	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.format = depthFormat;
	imageInfo.extent.width = Vulkan_Data.Width;
	imageInfo.extent.height = Vulkan_Data.Height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	imageInfo.queueFamilyIndexCount = 0;
	imageInfo.pQueueFamilyIndices = NULL;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.flags = 0;

	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = VK_NULL_HANDLE;
	viewInfo.format = depthFormat;
	viewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
	viewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
	viewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
	viewInfo.components.a = VK_COMPONENT_SWIZZLE_A;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.flags = 0;

	res = Vulkan_Functions.vkCreateImage(Vulkan_Data.Device, &imageInfo, NULL, &Vulkan_Data.Depth.Image);

	if (res != VK_SUCCESS)
	{
		ExitOnError("Failed to create depth image\n");

		return VkResult::VK_INCOMPLETE;
	}	

	VkMemoryRequirements memReq = {};
	Vulkan_Functions.vkGetImageMemoryRequirements(Vulkan_Data.Device, Vulkan_Data.Depth.Image, &memReq);

	VkMemoryAllocateInfo memoryAllocInfo = {};
	memoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocInfo.allocationSize = memReq.size;
	bool success = GetMemoryTypeFromProperties(Vulkan_Data
												, memReq.memoryTypeBits
												, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
												, &memoryAllocInfo.memoryTypeIndex);

	if (!success)
	{
		ExitOnError("Failed to get memory type\n");

		return VkResult::VK_INCOMPLETE;
	}

	res = Vulkan_Functions.vkAllocateMemory(Vulkan_Data.Device, &memoryAllocInfo, NULL, &Vulkan_Data.Depth.Memory);

	if (res != VK_SUCCESS)
	{
		ExitOnError("Failed to allocate memory\n");

		return VkResult::VK_INCOMPLETE;
	}

	res = Vulkan_Functions.vkBindImageMemory(Vulkan_Data.Device, Vulkan_Data.Depth.Image, Vulkan_Data.Depth.Memory, 0);

	if (res != VK_SUCCESS)
	{
		ExitOnError("Failed to bind memory to image\n");

		return VkResult::VK_INCOMPLETE;
	}

	viewInfo.image = Vulkan_Data.Depth.Image;
	res = Vulkan_Functions.vkCreateImageView(Vulkan_Data.Device, &viewInfo, NULL, &Vulkan_Data.Depth.View);

	if (res != VK_SUCCESS)
	{
		ExitOnError("Failed to create image view\n");

		return VkResult::VK_INCOMPLETE;
	}

	return res;
}

void CalculateMVP(TransformMatrices &transform) 
{
	// TODO(KAI): Check if any of these need to be transposed
	transform.ProjectionMat = Mat4::Perspective(45.0f, 1.0f, 1.0f, 0.1f, 1000.0f);
	transform.ViewMat = Mat4::LookAt(vec3f(-5, 3, -10), vec3f(), vec3f(0, -1, 0));
	transform.ModelMat = mat4f();

	// NOTE(KAI): https://matthewwellings.com/blog/the-new-vulkan-coordinate-system/
	transform.CorrectionMat = mat4f();
	transform.CorrectionMat[0] = vec4f(1,  0,    0, 0);
	transform.CorrectionMat[1] = vec4f(0, -1,    0, 0);
	transform.CorrectionMat[2] = vec4f(0,  0, 0.5f, 0);
	transform.CorrectionMat[3] = vec4f(0,  0, 0.5f, 1);

	transform.MVP = transform.CorrectionMat * transform.ProjectionMat * transform.ViewMat * transform.ModelMat;
}

///*NOTE(KAI)*///
/// To create uniform buffer we need:
/// 1- The data we will put in the buffer (in this case the MVP matrix)
/// 2- The buffer
/// 3- The memory
/// 4- Put the data in the memory by: mapping the memory, copying the data to the memory then unmaping the memory
/// 5- bind the memory to the buffer
VkResult InitUniformBuffer() 
{
	CalculateMVP(Vulkan_Data.Transform);

	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	bufferInfo.size = sizeof(Vulkan_Data.Transform.MVP);
	bufferInfo.queueFamilyIndexCount = 0;
	bufferInfo.pQueueFamilyIndices = NULL;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferInfo.flags = 0;

	VkResult res = Vulkan_Functions.vkCreateBuffer(Vulkan_Data.Device, &bufferInfo, NULL, &Vulkan_Data.UniformBuffer.Buffer);

	if (res != VK_SUCCESS)
	{
		ExitOnError("Failed to create buffer\n");

		return VkResult::VK_INCOMPLETE;
	}

	VkMemoryRequirements memReq = {};
	Vulkan_Functions.vkGetBufferMemoryRequirements(Vulkan_Data.Device, Vulkan_Data.UniformBuffer.Buffer, &memReq);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memReq.size;

	bool success = GetMemoryTypeFromProperties(Vulkan_Data
											, memReq.memoryTypeBits
											, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
											, &allocInfo.memoryTypeIndex);

	if (!success)
	{
		ExitOnError("Failed to get memory type for buffer memory\n");

		return VkResult::VK_INCOMPLETE;
	}

	res = Vulkan_Functions.vkAllocateMemory(Vulkan_Data.Device, &allocInfo, NULL, &Vulkan_Data.UniformBuffer.Memory);

	if (res != VK_SUCCESS)
	{
		ExitOnError("Failed to allocate buffer memory\n");

		return VkResult::VK_INCOMPLETE;
	}

	uint8_t *data;
	res = Vulkan_Functions.vkMapMemory(Vulkan_Data.Device, Vulkan_Data.UniformBuffer.Memory, 0, memReq.size, 0, (void**)&data);

	if (res != VK_SUCCESS)
	{
		ExitOnError("Failed to map buffer memory\n");

		return VkResult::VK_INCOMPLETE;
	}

	memcpy(data, &Vulkan_Data.Transform.MVP, sizeof(Vulkan_Data.Transform.MVP));

	Vulkan_Functions.vkUnmapMemory(Vulkan_Data.Device, Vulkan_Data.UniformBuffer.Memory);

	res = Vulkan_Functions.vkBindBufferMemory(Vulkan_Data.Device, Vulkan_Data.UniformBuffer.Buffer, Vulkan_Data.UniformBuffer.Memory, 0);

	if (res != VK_SUCCESS)
	{
		ExitOnError("Failed to bind buffer memory\n");

		return VkResult::VK_INCOMPLETE;
	}

	Vulkan_Data.UniformBuffer.BufferInfo.buffer = Vulkan_Data.UniformBuffer.Buffer;
	Vulkan_Data.UniformBuffer.BufferInfo.offset = 0;
	Vulkan_Data.UniformBuffer.BufferInfo.range = sizeof(Vulkan_Data.Transform.MVP);

	return res;
}

#define NUM_OF_DESCRIPTOR_SETS 1

///*NOTE(KAI)*///
/// Shaders need descriptors to access uniforms
/// Descriptors need to belong to descriptor set
/// Descriptor sets need to belong to a descriptor set layout (which is used to describe a list of descriptor sets)
/// Each descriptor set needs to have a layout binding (used to describe the descriptor set)
/// Pipeline layout is used to describe a list of descriptor set layouts
VkResult InitPipelineLayout() 
{
	VkDescriptorSetLayoutBinding layoutBinding = {};
	layoutBinding.binding = 0;
	layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layoutBinding.descriptorCount = 1;
	layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	
	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &layoutBinding;

	Vulkan_Data.DescriptorSetLayouts.resize(NUM_OF_DESCRIPTOR_SETS);

	VkResult res = Vulkan_Functions.vkCreateDescriptorSetLayout(Vulkan_Data.Device, &layoutInfo, NULL, Vulkan_Data.DescriptorSetLayouts.data());

	if (res != VK_SUCCESS)
	{
		ExitOnError("Failed to create descriptor set layout\n");

		return VkResult::VK_INCOMPLETE;
	}

	VkPipelineLayoutCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineInfo.setLayoutCount = NUM_OF_DESCRIPTOR_SETS;
	pipelineInfo.pSetLayouts = Vulkan_Data.DescriptorSetLayouts.data();

	res = Vulkan_Functions.vkCreatePipelineLayout(Vulkan_Data.Device, &pipelineInfo, NULL, &Vulkan_Data.PipelineLayout);

	if (res != VK_SUCCESS)
	{
		ExitOnError("Failed to create pipeline layout\n");

		return VkResult::VK_INCOMPLETE;
	}

	return res;
}

///*NOTE(KAI)*///
/// To allocate memory for descriptor set we need:
/// 1- Descriptor pool, one pool for each descriptor type (we only have one)
/// 2- Allocate memory for the descriptor
/// 3- Copy data from the uniform buffer to the descriptor
VkResult InitDescriptorSet() 
{
	VkDescriptorPoolSize typeCount[1];
	typeCount[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	typeCount[0].descriptorCount = 1;

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.maxSets = 1;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = typeCount;

	VkResult res = Vulkan_Functions.vkCreateDescriptorPool(Vulkan_Data.Device, &poolInfo, NULL, &Vulkan_Data.DescriptorPool);

	if (res != VK_SUCCESS)
	{
		ExitOnError("Failed to create descriptor pool\n");

		return VkResult::VK_INCOMPLETE;
	}

	VkDescriptorSetAllocateInfo allocInfo[1];
	allocInfo[0] = {};
	allocInfo[0].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo[0].descriptorSetCount = NUM_OF_DESCRIPTOR_SETS;
	allocInfo[0].descriptorPool = Vulkan_Data.DescriptorPool;
	allocInfo[0].pSetLayouts = Vulkan_Data.DescriptorSetLayouts.data();

	Vulkan_Data.DescriptorSets.resize(NUM_OF_DESCRIPTOR_SETS);

	res = Vulkan_Functions.vkAllocateDescriptorSets(Vulkan_Data.Device, allocInfo, Vulkan_Data.DescriptorSets.data());

	if (res != VK_SUCCESS)
	{
		ExitOnError("Failed to allocate memory for descriptor set from the pool\n");

		return VkResult::VK_INCOMPLETE;
	}

	VkWriteDescriptorSet writes[1];
	writes[0] = {};
	writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writes[0].dstSet = Vulkan_Data.DescriptorSets[0];
	writes[0].descriptorCount = 1;
	writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writes[0].pBufferInfo = &Vulkan_Data.UniformBuffer.BufferInfo;
	writes[0].dstArrayElement = 0;
	writes[0].dstBinding = 0;

	Vulkan_Functions.vkUpdateDescriptorSets(Vulkan_Data.Device, 1, writes, 0, NULL);

	return VkResult::VK_SUCCESS;
}

///*NOTE(KAI)*///
/// To create render pass we need:
/// 1- Create a semaphore for aquiring an image from the swapchain and prepare it for the render pass
/// 2- Aquiring the image
/// 3- Prepare color and depth attachment by specifying the layout transition
/// 4- Describe the render subpass
/// 5- Create the render pass
VkResult InitRenderPass() 
{
	VkSemaphore imageAquiredSemaphore;
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	
	VkResult res = Vulkan_Functions.vkCreateSemaphore(Vulkan_Data.Device, &semaphoreInfo, NULL, &imageAquiredSemaphore);

	if (res != VK_SUCCESS)
	{
		ExitOnError("Failed to create semaphore \n");

		return VkResult::VK_INCOMPLETE;
	}

	res = Vulkan_Functions.vkAcquireNextImageKHR(Vulkan_Data.Device
												, Vulkan_Data.SwapChain
												, UINT64_MAX
												, imageAquiredSemaphore
												, VK_NULL_HANDLE
												, &Vulkan_Data.CurrentBuffer);

	if (res != VK_SUCCESS)
	{
		ExitOnError("Failed to aquire the swapchain image\n");

		return VkResult::VK_INCOMPLETE;
	}

	VkAttachmentDescription attachments[2];
	attachments[0] = {};
	attachments[0].format = Vulkan_Data.Format;
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	attachments[1] = {};
	attachments[1].format = Vulkan_Data.Depth.Format;
	attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorRef = {};
	colorRef.attachment = 0;
	colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthRef = {};
	depthRef.attachment = 1;
	depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorRef;
	subpass.pDepthStencilAttachment = &depthRef;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 2;
	renderPassInfo.pAttachments = attachments;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	res = Vulkan_Functions.vkCreateRenderPass(Vulkan_Data.Device, &renderPassInfo, NULL, &Vulkan_Data.RenderPass);

	if (res != VK_SUCCESS)
	{
		ExitOnError("Failed to create render pass\n");

		return VkResult::VK_INCOMPLETE;
	}

	return res;
}

///*NOTE(KAI)*///
/// To create a shader we need:
/// 1- The shader code in GLSL (here we have vertex and fragment shaders only)
/// 2- An array of unsigned int to hold the converted shader code
/// 3- Shader stages info
/// 4- The GLSL to SPIR-V conversion functions
/// 5- The shader modules
/// 6- We convert the GLSL to SPIR-V
/// 7- Fill-in the shader stages info
/// 8- Then we create the shader modules using the converted code and the shader stages
VkResult InitShader() 
{
	const char *vertText =
		"#version 400\n"
		"#extension GL_ARB_separate_shader_objects : enable\n"
		"#extension GL_ARB_shading_language_420pack : enable\n"
		"layout (std140, binding = 0) uniform bufferVals\n"
		"{\n"
		"	mat4 MVP;\n"
		"} myBufferVals;\n"
		"layout (location = 0) in vec4 pos;\n"
		"layout (location = 1) in vec4 inColor;\n"
		"layout (location = 0) out vec4 outColor;\n"
		"void main()\n"
		"{\n"
		"	outColor = inColor;\n"
		"	gl_Position = myBufferVals.MVP * pos;\n"
		"}\n";

	const char *fragText =
		"#version 400\n"
		"#extension GL_ARB_separate_shader_objects : enable\n"
		"#extension GL_ARB_shading_language_420pack : enable\n"
		"layout (location = 0) in vec4 color;\n"
		"layout (location = 0) out vec4 outColor;\n"
		"void main()\n"
		"{\n"
		"	outColor = color;\n"
		"}\n";
		
	std::vector<unsigned int>  vertToSPRV;
	Vulkan_Data.ShaderStages[0] = {};
	Vulkan_Data.ShaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	Vulkan_Data.ShaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	Vulkan_Data.ShaderStages[0].pName = "main";

	std::vector<unsigned int>  fragToSPRV;
	Vulkan_Data.ShaderStages[1] = {};
	Vulkan_Data.ShaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	Vulkan_Data.ShaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	Vulkan_Data.ShaderStages[1].pName = "main";

	glslang::InitializeProcess();

	GLSLtoSPV(VK_SHADER_STAGE_VERTEX_BIT, vertText, vertToSPRV);
	GLSLtoSPV(VK_SHADER_STAGE_FRAGMENT_BIT, fragText, fragToSPRV);

	VkShaderModuleCreateInfo vertModInfo = {};
	vertModInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	vertModInfo.codeSize = vertToSPRV.size() * sizeof(unsigned int);
	vertModInfo.pCode = vertToSPRV.data();

	VkResult res = Vulkan_Functions.vkCreateShaderModule(Vulkan_Data.Device, &vertModInfo, NULL, &Vulkan_Data.ShaderStages[0].module);

	if (res != VK_SUCCESS)
	{
		ExitOnError("Failed to created vertex shader module\n");

		return VkResult::VK_INCOMPLETE;
	}

	VkShaderModuleCreateInfo fragModInfo = {};
	fragModInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	fragModInfo.codeSize = fragToSPRV.size() * sizeof(unsigned int);
	fragModInfo.pCode = fragToSPRV.data();

	res = Vulkan_Functions.vkCreateShaderModule(Vulkan_Data.Device, &fragModInfo, NULL, &Vulkan_Data.ShaderStages[1].module);

	if (res != VK_SUCCESS)
	{
		ExitOnError("Failed to created fragment shader module\n");

		return VkResult::VK_INCOMPLETE;
	}

	glslang::FinalizeProcess();

	return res;
}

int InitVulkan()
{
	HMODULE vkLibrary = LoadLibrary("vulkan-1.dll");

	if (vkLibrary)
	{
		Vulkan_Functions.vkGetInstanceProcAddr = 
			(PFN_vkGetInstanceProcAddr)GetProcAddress(vkLibrary, "vkGetInstanceProcAddr"); 
		Vulkan_Functions.vkGetDeviceProcAddr = 
			(PFN_vkGetDeviceProcAddr)GetProcAddress(vkLibrary, "vkGetDeviceProcAddr"); 
	}

	if (!Vulkan_Functions.vkGetInstanceProcAddr || !Vulkan_Functions.vkGetDeviceProcAddr)
	{
		return NULL;
	}

	Vulkan_Functions.vkCreateInstance = (PFN_vkCreateInstance)GetFunctionPointer(NULL, "vkCreateInstance");

	if (CreateVulkanInstance() != VK_SUCCESS)
	{
		ExitOnError("Failed to create vulkan instance\n");
		
		return NULL;
	}

	Vulkan_Functions.vkEnumeratePhysicalDevices = (PFN_vkEnumeratePhysicalDevices)GetFunctionPointer(Vulkan_Data.Instance, "vkEnumeratePhysicalDevices");
	Vulkan_Functions.vkGetPhysicalDeviceQueueFamilyProperties = (PFN_vkGetPhysicalDeviceQueueFamilyProperties)GetFunctionPointer(Vulkan_Data.Instance, "vkGetPhysicalDeviceQueueFamilyProperties");
	Vulkan_Functions.vkCreateDevice = (PFN_vkCreateDevice)GetFunctionPointer(Vulkan_Data.Instance, "vkCreateDevice");
	Vulkan_Functions.vkCreateCommandPool = (PFN_vkCreateCommandPool)GetFunctionPointer(Vulkan_Data.Instance, "vkCreateCommandPool");
	Vulkan_Functions.vkAllocateCommandBuffers = (PFN_vkAllocateCommandBuffers)GetFunctionPointer(Vulkan_Data.Instance, "vkAllocateCommandBuffers");
	Vulkan_Functions.vkGetPhysicalDeviceSurfaceSupportKHR = (PFN_vkGetPhysicalDeviceSurfaceSupportKHR)GetFunctionPointer(Vulkan_Data.Instance, "vkGetPhysicalDeviceSurfaceSupportKHR");
	Vulkan_Functions.vkCreateWin32SurfaceKHR = (PFN_vkCreateWin32SurfaceKHR)GetFunctionPointer(Vulkan_Data.Instance, "vkCreateWin32SurfaceKHR");
	Vulkan_Functions.vkGetPhysicalDeviceSurfaceFormatsKHR = (PFN_vkGetPhysicalDeviceSurfaceFormatsKHR)GetFunctionPointer(Vulkan_Data.Instance, "vkGetPhysicalDeviceSurfaceFormatsKHR");
	Vulkan_Functions.vkGetPhysicalDeviceSurfaceCapabilitiesKHR = (PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR)GetFunctionPointer(Vulkan_Data.Instance, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
	Vulkan_Functions.vkGetPhysicalDeviceSurfacePresentModesKHR = (PFN_vkGetPhysicalDeviceSurfacePresentModesKHR)GetFunctionPointer(Vulkan_Data.Instance, "vkGetPhysicalDeviceSurfacePresentModesKHR");
	Vulkan_Functions.vkCreateSwapchainKHR = (PFN_vkCreateSwapchainKHR)GetFunctionPointer(Vulkan_Data.Instance, "vkCreateSwapchainKHR");
	Vulkan_Functions.vkGetSwapchainImagesKHR = (PFN_vkGetSwapchainImagesKHR)GetFunctionPointer(Vulkan_Data.Instance, "vkGetSwapchainImagesKHR");
	Vulkan_Functions.vkCreateImageView = (PFN_vkCreateImageView)GetFunctionPointer(Vulkan_Data.Instance, "vkCreateImageView");
	Vulkan_Functions.vkCreateImage = (PFN_vkCreateImage)GetFunctionPointer(Vulkan_Data.Instance, "vkCreateImage");
	Vulkan_Functions.vkGetPhysicalDeviceFormatProperties = (PFN_vkGetPhysicalDeviceFormatProperties)GetFunctionPointer(Vulkan_Data.Instance, "vkGetPhysicalDeviceFormatProperties");
	Vulkan_Functions.vkGetImageMemoryRequirements = (PFN_vkGetImageMemoryRequirements)GetFunctionPointer(Vulkan_Data.Instance, "vkGetImageMemoryRequirements");
	Vulkan_Functions.vkGetPhysicalDeviceProperties = (PFN_vkGetPhysicalDeviceProperties)GetFunctionPointer(Vulkan_Data.Instance, "vkGetPhysicalDeviceProperties");
	Vulkan_Functions.vkGetPhysicalDeviceMemoryProperties = (PFN_vkGetPhysicalDeviceMemoryProperties)GetFunctionPointer(Vulkan_Data.Instance, "vkGetPhysicalDeviceMemoryProperties");
	Vulkan_Functions.vkAllocateMemory = (PFN_vkAllocateMemory)GetFunctionPointer(Vulkan_Data.Instance, "vkAllocateMemory");
	Vulkan_Functions.vkBindImageMemory = (PFN_vkBindImageMemory)GetFunctionPointer(Vulkan_Data.Instance, "vkBindImageMemory");
	Vulkan_Functions.vkCreateBuffer = (PFN_vkCreateBuffer)GetFunctionPointer(Vulkan_Data.Instance, "vkCreateBuffer");
	Vulkan_Functions.vkGetBufferMemoryRequirements = (PFN_vkGetBufferMemoryRequirements)GetFunctionPointer(Vulkan_Data.Instance, "vkGetBufferMemoryRequirements");
	Vulkan_Functions.vkMapMemory = (PFN_vkMapMemory)GetFunctionPointer(Vulkan_Data.Instance, "vkMapMemory");
	Vulkan_Functions.vkUnmapMemory = (PFN_vkUnmapMemory)GetFunctionPointer(Vulkan_Data.Instance, "vkUnmapMemory");
	Vulkan_Functions.vkBindBufferMemory = (PFN_vkBindBufferMemory)GetFunctionPointer(Vulkan_Data.Instance, "vkBindBufferMemory");
	Vulkan_Functions.vkCreateDescriptorSetLayout = (PFN_vkCreateDescriptorSetLayout)GetFunctionPointer(Vulkan_Data.Instance, "vkCreateDescriptorSetLayout");
	Vulkan_Functions.vkCreatePipelineLayout = (PFN_vkCreatePipelineLayout)GetFunctionPointer(Vulkan_Data.Instance, "vkCreatePipelineLayout");
	Vulkan_Functions.vkCreateDescriptorPool = (PFN_vkCreateDescriptorPool)GetFunctionPointer(Vulkan_Data.Instance, "vkCreateDescriptorPool");
	Vulkan_Functions.vkAllocateDescriptorSets = (PFN_vkAllocateDescriptorSets)GetFunctionPointer(Vulkan_Data.Instance, "vkAllocateDescriptorSets");
	Vulkan_Functions.vkUpdateDescriptorSets = (PFN_vkUpdateDescriptorSets)GetFunctionPointer(Vulkan_Data.Instance, "vkUpdateDescriptorSets");
	Vulkan_Functions.vkCreateSemaphore = (PFN_vkCreateSemaphore)GetFunctionPointer(Vulkan_Data.Instance, "vkCreateSemaphore");
	Vulkan_Functions.vkAcquireNextImageKHR = (PFN_vkAcquireNextImageKHR)GetFunctionPointer(Vulkan_Data.Instance, "vkAcquireNextImageKHR");
	Vulkan_Functions.vkCreateRenderPass = (PFN_vkCreateRenderPass)GetFunctionPointer(Vulkan_Data.Instance, "vkCreateRenderPass");
	Vulkan_Functions.vkCreateShaderModule = (PFN_vkCreateShaderModule)GetFunctionPointer(Vulkan_Data.Instance, "vkCreateShaderModule");

	if (CreateVulkanDevice() != VK_SUCCESS)
	{
		ExitOnError("Failed to create vulkan physical device\n");
		
		return NULL;
	}

	if (InitCommandBufferPool() != VK_SUCCESS)
	{
		ExitOnError("Failed to create command buffer pooln\n");

		return NULL;
	}

	if (InitCommandBuffer() != VK_SUCCESS)
	{
		ExitOnError("Failed to create command buffer\n");

		return NULL;
	}

	if (InitSurface() != VK_SUCCESS)
	{
		ExitOnError("Failed to create surface\n");

		return NULL;
	}

	if (InitSwapChain() != VK_SUCCESS)
	{
		ExitOnError("Failed to create swapchain \n");

		return NULL;
	}

	if (InitDepthBuffer() != VK_SUCCESS)
	{
		ExitOnError("Failed to create depth buffer\n");

		return NULL;
	}

	if (InitUniformBuffer() != VK_SUCCESS)
	{
		ExitOnError("Failed to create uniform buffer\n");

		return NULL;
	}

	if (InitPipelineLayout() != VK_SUCCESS)
	{
		ExitOnError("Failed to create pipeline layout\n");

		return NULL;
	}

	if (InitDescriptorSet() != VK_SUCCESS)
	{
		ExitOnError("Failed to allocate memory for descriptor set\n");

		return NULL;
	}

	if (InitRenderPass() != VK_SUCCESS)
	{
		ExitOnError("Failed to create render pass\n");

		return NULL;
	}

	if (InitShader() != VK_SUCCESS)
	{
		ExitOnError("Failed to create shader\n");

		return NULL;
	}

	return 1;
}

LRESULT CALLBACK WindowCallBack(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;

	result = DefWindowProc(window, message, wParam, lParam);

	return result;
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR cmdLine, int cmdShow)
{
	WNDCLASS windowClass = {};

	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.hInstance = hInstance;
	windowClass.lpszClassName = "VulkanTest";
	windowClass.lpfnWndProc = WindowCallBack;

	bool isRunning = false;	

	Vulkan_Data.Width = 1280;
	Vulkan_Data.Height = 720;

	if (RegisterClass(&windowClass))
	{
		Vulkan_Data.Window = CreateWindowEx(NULL
			, windowClass.lpszClassName
			, "Vulkan Test"
			, WS_OVERLAPPEDWINDOW | WS_VISIBLE
			, CW_USEDEFAULT
			, CW_USEDEFAULT
			, Vulkan_Data.Width
			, Vulkan_Data.Height
			, NULL
			, NULL
			, hInstance
			, NULL);

		if (Vulkan_Data.Window)
		{
			if (!InitVulkan())
			{
				ExitOnError("Failed to initialize vulkan\n");

				return 0;
			}

			isRunning = true;
			while (isRunning)
			{
				MSG message;

				while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
				{
					if (message.message == WM_QUIT)
					{
						isRunning = false;
					}

					TranslateMessage(&message);
					DispatchMessage(&message);
				}
			}
		}
		else
		{
			ExitOnError("Couldn't create Window\n");
		}
	}
	else
	{
		ExitOnError("Couldn't register window class\n");
	}

	system("pause");

	return 0;
}

int main()
{
	WinMain(0, 0, 0, 0);

	return 0;
}