#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>

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

    }

    void mainLoop () {
		// 主循环，直到窗口关闭
        while (!glfwWindowShouldClose (window)) {
			// 处理窗口事件
            glfwPollEvents ();
        }
    }

    void cleanup () {
		// 销毁窗口和终止GLFW
        glfwDestroyWindow (window);

        glfwTerminate ();
    }

private:
    GLFWwindow* window;
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