#define NOMINMAX

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>

#include <vector>
#include <optional>
#include <cstdint> // Necessary for uint32_t
#include <limits> // Necessary for std::numeric_limits
#include <algorithm> // Necessary for std::clamp

#include <set>
#include <fstream>

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete () {
        return graphicsFamily.has_value () && presentFamily.has_value ();
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class HelloTriangleApplication {

    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;

    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

    const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif
public:
    void run () {
        initWindow ();
        initVulkan ();
        mainLoop ();
        cleanup ();
    }

private:
    static std::vector<char> readFile (const std::string& filename) {
        std::ifstream file (filename, std::ios::ate | std::ios::binary);

        if (!file.is_open ()) {
            throw std::runtime_error ("failed to open file!");
        }
		// 读取文件大小并分配缓冲区
        size_t fileSize = (size_t)file.tellg ();
        std::vector<char> buffer (fileSize);
		// 将文件指针移动到文件开头并读取文件内容到缓冲区
        file.seekg (0);
        file.read (buffer.data (), fileSize);
        file.close ();

        return buffer;
    }

    bool checkValidationLayerSupport () {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties (&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers (layerCount);
        vkEnumerateInstanceLayerProperties (&layerCount, availableLayers.data ());

        for (const char* layerName : validationLayers) {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers) {
                if (strcmp (layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
                return false;
            }
        }

        return true;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback (
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {
        
        if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
            // Message is important enough to show
            std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
        }

        return VK_FALSE;
    }

    void populateDebugMessengerCreateInfo (VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
        // 填充调试信使创建信息
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.flags = 0;   // Padding bit
        // 我们希望接收的消息严重性和类型
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        // 设置回调函数
        createInfo.pfnUserCallback = debugCallback;
        createInfo.pUserData = nullptr; // Optional
    }

	// ========================================= 由于是扩展函数，所以需要手动加载 =========================================
    VkResult CreateDebugUtilsMessengerEXT (VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr (instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) {
            return func (instance, pCreateInfo, pAllocator, pDebugMessenger);
        }
        else {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    void DestroyDebugUtilsMessengerEXT (VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr (instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func (instance, debugMessenger, pAllocator);
        }
    }
	// ========================================= 由于是扩展函数，所以需要手动加载 =========================================


    SwapChainSupportDetails querySwapChainSupport (VkPhysicalDevice device) {
        SwapChainSupportDetails details;
        // 查询物理设备的表面能力
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR (device, surface, &details.capabilities);

        // 查询物理设备的表面格式
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR (device, surface, &formatCount, nullptr);

        if (formatCount != 0) {
            details.formats.resize (formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR (device, surface, &formatCount, details.formats.data ());
        }

        // 查询物理设备的表面呈现模式
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR (device, surface, &presentModeCount, nullptr);

        if (presentModeCount != 0) {
            details.presentModes.resize (presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR (device, surface, &presentModeCount, details.presentModes.data ());
        }
        return details;
    }

	// 选择交换链表面格式
    VkSurfaceFormatKHR chooseSwapSurfaceFormat (const std::vector<VkSurfaceFormatKHR>& availableFormats) {
		// 如果格式是 VK_FORMAT_B8G8R8A8_SRGB 且颜色空间是 VK_COLOR_SPACE_SRGB_NONLINEAR_KHR，则选择该格式
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }

		// 否则，返回第一个可用的格式
        return availableFormats[0];
    }
	// 选择交换链呈现模式
    VkPresentModeKHR chooseSwapPresentMode (const std::vector<VkPresentModeKHR>& availablePresentModes) {
		// 如果支持 VK_PRESENT_MODE_MAILBOX_KHR，则选择该模式
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
        }
		// 否则，返回 VK_PRESENT_MODE_FIFO_KHR，这是唯一保证可用的模式
        return VK_PRESENT_MODE_FIFO_KHR;
    }
	// 选择交换链扩展
    VkExtent2D chooseSwapExtent (const VkSurfaceCapabilitiesKHR& capabilities) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max ()) {
            return capabilities.currentExtent;
        }
        else {
            int width, height;
            glfwGetFramebufferSize (window, &width, &height);

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            actualExtent.width = std::clamp (actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp (actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }

    std::vector<const char*> getRequiredExtensions () {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions (&glfwExtensionCount);

        std::vector<const char*> extensions (glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (enableValidationLayers) {
            extensions.push_back (VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        // 查询Vulkan支持的扩展
        uint32_t vk_extensionCount = 0;
        vkEnumerateInstanceExtensionProperties (nullptr, &vk_extensionCount, nullptr);
        std::vector<VkExtensionProperties> vk_extensions (vk_extensionCount);
        vkEnumerateInstanceExtensionProperties (nullptr, &vk_extensionCount, vk_extensions.data ());
        // 输出可用的扩展列表
        std::cout << "available vk_extensions:\n";
        for (const auto& vk_extension : vk_extensions) {
            std::cout << '\t' << vk_extension.extensionName << '\n';
        }

        return extensions;
    }

    void createInstance () {
		// 检查验证层是否可用
        if (enableValidationLayers && !checkValidationLayerSupport ()) {
            throw std::runtime_error ("validation layers requested, but not available!");
        }

		// 填写应用程序信息
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION (1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION (1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

		// 创建Vulkan实例Info
        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

		// 返回GLFW所需的Vk扩展列表
        auto extensions = getRequiredExtensions ();
        // 设置Vk扩展信息
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size ());
        createInfo.ppEnabledExtensionNames = extensions.data ();

		// 设置验证层信息
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size ());
            createInfo.ppEnabledLayerNames = validationLayers.data ();

            populateDebugMessengerCreateInfo (debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
        }
        else {
            createInfo.enabledLayerCount = 0;

            createInfo.pNext = nullptr;
        }

		// 创建Vulkan实例
        if (vkCreateInstance (&createInfo, nullptr, &instance) != VK_SUCCESS) {
            throw std::runtime_error ("failed to create instance!");
        }
    }

    void setupDebugMessenger () {
        if (!enableValidationLayers) return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
        populateDebugMessengerCreateInfo (createInfo);

        if (CreateDebugUtilsMessengerEXT (instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
            throw std::runtime_error ("failed to set up debug messenger!");
        }
    }

    void createSurface () {
        if (glfwCreateWindowSurface (instance, window, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error ("failed to create window surface!");
        }
    }

    QueueFamilyIndices findQueueFamilies (VkPhysicalDevice device) {
        QueueFamilyIndices indices;
        // Logic to find queue family indices to populate struct with

        // 获取物理设备的队列族数量
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties (device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies (queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties (device, &queueFamilyCount, queueFamilies.data ());

        int i = 0;
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR (device, i, surface, &presentSupport);
            if (presentSupport) {
                indices.presentFamily = i;
            }

            if (indices.isComplete ()) {
                break;
            }

            i++;
        }

        return indices;
    }

    bool checkDeviceExtensionSupport (VkPhysicalDevice device) {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties (device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions (extensionCount);
        vkEnumerateDeviceExtensionProperties (device, nullptr, &extensionCount, availableExtensions.data ());

        std::set<std::string> requiredExtensions (deviceExtensions.begin (), deviceExtensions.end ());

        for (const auto& extension : availableExtensions) {
            requiredExtensions.erase (extension.extensionName);
        }

        return requiredExtensions.empty ();
    }

    bool isDeviceSuitable (VkPhysicalDevice device) {
		// 查询设备属性和特性
        VkPhysicalDeviceProperties deviceProperties;
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceProperties (device, &deviceProperties);
        vkGetPhysicalDeviceFeatures (device, &deviceFeatures);
		// 检查设备是否支持所需的队列族
        QueueFamilyIndices indices = findQueueFamilies (device);
		// 检测设备是否支持所需的扩展
        bool extensionsSupported = checkDeviceExtensionSupport (device);

        bool swapChainAdequate = false;
        if (extensionsSupported) {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport (device);
            swapChainAdequate = !swapChainSupport.formats.empty () && !swapChainSupport.presentModes.empty ();
        }

		// 只要设备支持图形队列族，支持所需的扩展，并且交换链足够，就认为设备是合适的
        return indices.isComplete () && extensionsSupported && swapChainAdequate;

        //return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
        //    deviceFeatures.geometryShader;
    }

    void pickPhysicalDevice () {
		// 获取物理设备数量
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices (instance, &deviceCount, nullptr);
        if (deviceCount == 0) {
            throw std::runtime_error ("failed to find GPUs with Vulkan support!");
        }
		// 获取物理设备列表
        std::vector<VkPhysicalDevice> devices (deviceCount);
        vkEnumeratePhysicalDevices (instance, &deviceCount, devices.data ());

        for (const auto& device : devices) {
            if (isDeviceSuitable (device)) {
                physicalDevice = device;
                break;
            }
        }

        if (physicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error ("failed to find a suitable GPU!");
        }
    }

    void createLogicalDevice () {
		// 获取物理设备的队列族索引
        QueueFamilyIndices indices = findQueueFamilies (physicalDevice);
		// 创建逻辑设备的队列创建信息
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value (), indices.presentFamily.value () };
        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
			// 设置队列优先级
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back (queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};

		// 创建逻辑设备创建信息
        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		// 填入队列创建信息
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size ());
        createInfo.pQueueCreateInfos = queueCreateInfos.data ();
		// 填入设备特性
        createInfo.pEnabledFeatures = &deviceFeatures;

        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size ());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data ();
        createInfo.enabledLayerCount = 0;
       //     if (enableValidationLayers) {
			    //// 启动验证层
       //         createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size ());
       //         createInfo.ppEnabledLayerNames = validationLayers.data ();
       //     }
       //     else {
       //         createInfo.enabledLayerCount = 0;
       //     }
		// 创建逻辑设备
        if (vkCreateDevice (physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
            throw std::runtime_error ("failed to create logical device!");
        }

        vkGetDeviceQueue (device, indices.graphicsFamily.value (), 0, &graphicsQueue);
        vkGetDeviceQueue (device, indices.presentFamily.value (), 0, &presentQueue);
    }

    void createSwapChain () {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport (physicalDevice);
		// 选择交换链表面格式、呈现模式和扩展
        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat (swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode (swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent (swapChainSupport.capabilities);
		// 计算交换链图像数量
        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		// 确保交换链图像数量不超过最大值
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;

        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = findQueueFamilies (physicalDevice);
        uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value (), indices.presentFamily.value () };
		// 如果图形队列族和呈现队列族不同，则设置共享模式为并发模式
        if (indices.graphicsFamily != indices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0; // Optional
            createInfo.pQueueFamilyIndices = nullptr; // Optional
        }
		// 设置交换链的预变换
        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		// 设置交换链的复合Alpha模式
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		// 设置交换链的呈现模式
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
		// 设置旧的交换链为空
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        if (vkCreateSwapchainKHR (device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
            throw std::runtime_error ("failed to create swap chain!");
        }

		// 获取交换链图像
        vkGetSwapchainImagesKHR (device, swapChain, &imageCount, nullptr);
        swapChainImages.resize (imageCount);
        vkGetSwapchainImagesKHR (device, swapChain, &imageCount, swapChainImages.data ());

		// 保存交换链图像格式和范围
        swapChainImageFormat = surfaceFormat.format;
        swapChainExtent = extent;
    }

    void createImageViews () {
        swapChainImageViews.resize (swapChainImages.size ());
        for (size_t i = 0; i < swapChainImages.size (); i++) {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = swapChainImages[i];
			// 视图类型为2D纹理
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = swapChainImageFormat;
			// 设置颜色通道映射
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			// 设置子资源范围
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView (device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
                throw std::runtime_error ("failed to create image views!");
            }
        }
    }

    void createRenderPass () {
		// 创建渲染通道的颜色附件描述
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = swapChainImageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		// 设置加载操作为清除，存储操作为存储
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		// 设置模板加载和存储操作为不关心
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		// 设置初始布局为未定义，最终布局为呈现源
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		// 创建渲染通道的颜色附件引用
        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		// 创建子通道描述
        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		// 设置子通道的颜色附件引用
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;

        if (vkCreateRenderPass (device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
            throw std::runtime_error ("failed to create render pass!");
        }
    }

	// 创建着色器模块
    VkShaderModule createShaderModule (const std::vector<char>& code) {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size ();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data ());
		// 创建着色器模块
        VkShaderModule shaderModule;
        if (vkCreateShaderModule (device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            throw std::runtime_error ("failed to create shader module!");
        }
        return shaderModule;
    }
	// 创建图形管线
    void createGraphicsPipeline () {
        auto vertShaderCode = readFile ("shaders/vert.spv");
        auto fragShaderCode = readFile ("shaders/frag.spv");

        VkShaderModule vertShaderModule = createShaderModule (vertShaderCode);
        VkShaderModule fragShaderModule = createShaderModule (fragShaderCode);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

		// 创建图形管线的着色器阶段信息数组
        VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		// 创建顶点输入状态信息
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 0;
        vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
        vertexInputInfo.vertexAttributeDescriptionCount = 0;
        vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

		// 创建输入汇编状态信息
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

		// 创建视口信息
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)swapChainExtent.width;
        viewport.height = (float)swapChainExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = swapChainExtent;

        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };

        // 创建动态状态信息
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size ());
        dynamicState.pDynamicStates = dynamicStates.data ();

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

		// 创建光栅化状态信息
        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;
        rasterizer.depthBiasConstantFactor = 0.0f; // Optional
        rasterizer.depthBiasClamp = 0.0f; // Optional
        rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

		// 创建多重采样状态信息
        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampling.minSampleShading = 1.0f; // Optional
        multisampling.pSampleMask = nullptr; // Optional
        multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
        multisampling.alphaToOneEnable = VK_FALSE; // Optional

		// 创建颜色混合状态信息
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

		// 创建颜色混合状态信息
        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f; // Optional
        colorBlending.blendConstants[1] = 0.0f; // Optional
        colorBlending.blendConstants[2] = 0.0f; // Optional
        colorBlending.blendConstants[3] = 0.0f; // Optional

		// 创建管线布局信息
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0; // Optional
        pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
        pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
        pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

        if (vkCreatePipelineLayout (device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error ("failed to create pipeline layout!");
        }

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
		// 设置图形管线的各个状态信息
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = nullptr; // Optional
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
		// 设置图形管线布局和渲染通道
        pipelineInfo.layout = pipelineLayout;
        pipelineInfo.renderPass = renderPass;
        pipelineInfo.subpass = 0;
		// 设置图形管线的基础管线信息
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
        pipelineInfo.basePipelineIndex = -1; // Optional

        if (vkCreateGraphicsPipelines (device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
            throw std::runtime_error ("failed to create graphics pipeline!");
        }

        vkDestroyShaderModule (device, fragShaderModule, nullptr);
        vkDestroyShaderModule (device, vertShaderModule, nullptr);
    }

    void createFramebuffers () {
		// 分配交换链帧缓冲列表的大小
        swapChainFramebuffers.resize (swapChainImageViews.size ());

        for (size_t i = 0; i < swapChainImageViews.size (); i++) {
            VkImageView attachments[] = {
                swapChainImageViews[i]
            };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = swapChainExtent.width;
            framebufferInfo.height = swapChainExtent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer (device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error ("failed to create framebuffer!");
            }
        }
    }

    // ---------------------------------------------
    void initWindow () {
		// 初始化GLFW库
        glfwInit ();
		// 设置GLFW窗口属性
        glfwWindowHint (GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint (GLFW_RESIZABLE, GLFW_FALSE);
		// 创建GLFW窗口
        window = glfwCreateWindow (WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    }

    void initVulkan () {
        createInstance ();
        setupDebugMessenger ();
        createSurface ();
        pickPhysicalDevice ();
        createLogicalDevice ();
        createSwapChain ();
        createImageViews ();
        createRenderPass ();
        createGraphicsPipeline ();
        createFramebuffers ();
    }

    void mainLoop () {
		// 主循环，直到窗口关闭
        while (!glfwWindowShouldClose (window)) {
			// 处理窗口事件
            glfwPollEvents ();
        }
    }

    void cleanup () {
		// 销毁交换链帧缓冲
        for (auto framebuffer : swapChainFramebuffers) {
            vkDestroyFramebuffer (device, framebuffer, nullptr);
        }
		// 销毁图形管线
        vkDestroyPipeline (device, graphicsPipeline, nullptr);
		// 销毁图形管线布局
        vkDestroyPipelineLayout (device, pipelineLayout, nullptr);
		// 销毁图形管线
        vkDestroyRenderPass (device, renderPass, nullptr);
		// 销毁交换链图像视图
        for (auto imageView : swapChainImageViews) {
            vkDestroyImageView (device, imageView, nullptr);
        }
		// 销毁交换链
        vkDestroySwapchainKHR (device, swapChain, nullptr);
		// 销毁逻辑设备
        vkDestroyDevice (device, nullptr);

		// 销毁调试信使
        if (enableValidationLayers) {
            DestroyDebugUtilsMessengerEXT (instance, debugMessenger, nullptr);
        }

		// 销毁窗口表面
        vkDestroySurfaceKHR (instance, surface, nullptr);
		// 销毁Vulkan实例
        vkDestroyInstance (instance, nullptr);
		// 销毁窗口和终止GLFW
        glfwDestroyWindow (window);

        glfwTerminate ();
    }

private:
	// GLFW窗口指针
    GLFWwindow* window;
	// Vulkan实例
    VkInstance instance;
	// Vulkan调试信息回调
    VkDebugUtilsMessengerEXT debugMessenger;
	// Vulkan物理设备
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	// Vulkan逻辑设备
    VkDevice device;
	// Vulkan图形队列 from logical device
    VkQueue graphicsQueue;
	// Vulkan呈现队列 from logical device
    VkQueue presentQueue;
    // Vulkan窗口表面
    VkSurfaceKHR surface;
	// Vulkan交换链
    VkSwapchainKHR swapChain;
	// Vulkan交换链图像列表
    std::vector<VkImage> swapChainImages;
	// Vulkan交换链图像格式
    VkFormat swapChainImageFormat;
	// Vulkan交换链图像范围
    VkExtent2D swapChainExtent;
	// Vulkan交换链图像视图列表
    std::vector<VkImageView> swapChainImageViews;
    // Vulkan交换链帧缓冲列表
    std::vector<VkFramebuffer> swapChainFramebuffers;
	// Vulkan图形管线布局与渲染Pass相关
    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
	// Vulkan图形管线
    VkPipeline graphicsPipeline;
};

int main () {
    HelloTriangleApplication app;

    try {
        app.run ();
    }
    catch (const std::exception& e) {
        std::cerr << e.what () << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}