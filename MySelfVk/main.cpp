#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>

#include <vector>

class HelloTriangleApplication {

    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;
public:
    void run () {
        initWindow ();
        initVulkan ();
        mainLoop ();
        cleanup ();
    }

private:
    void createInstance () {
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
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions (&glfwExtensionCount);
		// 设置Vk扩展信息
        createInfo.enabledExtensionCount = glfwExtensionCount;
        createInfo.ppEnabledExtensionNames = glfwExtensions;
		// 暂时不需要启用任何验证层
        createInfo.enabledLayerCount = 0;
		// 创建Vulkan实例
        if (vkCreateInstance (&createInfo, nullptr, &instance) != VK_SUCCESS) {
            throw std::runtime_error ("failed to create instance!");
        }

		// 查询Vulkan支持的扩展
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties (nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> extensions (extensionCount);
        vkEnumerateInstanceExtensionProperties (nullptr, &extensionCount, extensions.data ());
		// 输出可用的扩展列表
        std::cout << "available extensions:\n";
        for (const auto& extension : extensions) {
            std::cout << '\t' << extension.extensionName << '\n';
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
    }

    void mainLoop () {
		// 主循环，直到窗口关闭
        while (!glfwWindowShouldClose (window)) {
			// 处理窗口事件
            glfwPollEvents ();
        }
    }

    void cleanup () {
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