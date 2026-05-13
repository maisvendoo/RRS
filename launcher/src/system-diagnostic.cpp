#include    <system-diagnostic.h>
#include    <vulkan/vulkan.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
StateGPU getInfoGPUs(std::vector<gpu_info_t> &gpus_info)
{
    // Задаем информацию о приложении
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "RRS Launcher";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 9, 2);
    appInfo.pEngineName = "VulkanSceneGraph";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 1, 15);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    // Пытаемся создать vkInstance
    VkInstanceCreateInfo instanceCI{};
    instanceCI.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCI.pApplicationInfo = &appInfo;

    VkInstance instance = VK_NULL_HANDLE;
    VkResult result = vkCreateInstance(&instanceCI, nullptr, &instance);

    // Переустанови дарйвер с официального сайта!
    if (result != VK_SUCCESS)
    {
        return GPU_STATE_VK_INSTANCE_ERROR;
    }

    // Определяем наличие совместимых GPU в принципе
    uint32_t deviceCount = 0;
    result = vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    // Переустанови дарйвер с официального сайта!
    if (result != VK_SUCCESS && result != VK_INCOMPLETE)
    {
        return GPU_STATE_VK_ENUM_PHYSICAL_DEVICE_ERROR;
    }

    // Не повезло тебе с видюхой, чувак...
    if (deviceCount == 0)
    {
        return GPU_STATE_NO_CAPABLE_DEVICES_ERROR;
    }

    std::vector<VkPhysicalDevice> physicalDevices(deviceCount);

    result = vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data());

    if (result != VK_SUCCESS)
    {
        return GPU_STATE_GET_DEVICES_LIST_ERROR;
    }

    return GPU_STATE_READY;
}
