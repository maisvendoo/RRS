#include    <system-diagnostic.h>
#include    <vulkan/vulkan.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
const char* deviceTypeToStr(VkPhysicalDeviceType t)
{
    switch(t)
    {
    case VK_PHYSICAL_DEVICE_TYPE_OTHER:
        return "Other";

    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
        return "Integrated GPU";

    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
        return "Discrete GPU";

    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
        return "Virtual GPU";

    case VK_PHYSICAL_DEVICE_TYPE_CPU:
        return "CPU";

    default:
        return "Unknown";
    }
}

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
        // Купи новую, или играй в ZDSimulator :-)
        return GPU_STATE_NO_CAPABLE_DEVICES_ERROR;
    }

    std::vector<VkPhysicalDevice> physicalDevices(deviceCount);

    result = vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data());

    if (result != VK_SUCCESS)
    {
        return GPU_STATE_GET_DEVICES_LIST_ERROR;
    }

    // Смотрим что за видюхи нашлись
    for (size_t i = 0; i < deviceCount; ++i)
    {
        VkPhysicalDevice physDev = physicalDevices[i];

        // Цепочка pNext для Vulkan 1.1 + 1.2 + Core)
        VkPhysicalDeviceVulkan12Features vk12Feat{};
        vk12Feat.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;

        VkPhysicalDeviceVulkan11Features vk11Feat{};
        vk11Feat.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
        vk12Feat.pNext = &vk11Feat;

        VkPhysicalDeviceFeatures2 coreFeat2{};
        coreFeat2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        coreFeat2.pNext = &vk12Feat;

        vkGetPhysicalDeviceFeatures2(physDev, &coreFeat2);

        // Свойства (цепочка pNext)
        VkPhysicalDeviceVulkan12Properties vk12Prop{};
        vk12Prop.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES;

        VkPhysicalDeviceVulkan11Properties vk11Prop{};
        vk11Prop.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES;
        vk12Prop.pNext = &vk11Prop;

        VkPhysicalDeviceProperties2 coreProp2{};
        coreProp2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        coreProp2.pNext = &vk12Prop;

        vkGetPhysicalDeviceProperties2(physDev, &coreProp2);

        // Queue Families
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physDev, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physDev, &queueFamilyCount, queueFamilies.data());

        // Memory Properties
        VkPhysicalDeviceMemoryProperties memProps{};
        vkGetPhysicalDeviceMemoryProperties(physDev, &memProps);

        // Вычитываем информацию о карточках
        auto &props = coreProp2.properties;

        gpu_info_t gpu_info;
        gpu_info.deviceName = QString(props.deviceName);
        gpu_info.deviceType = QString(deviceTypeToStr(props.deviceType));
        gpu_info.vendorID = props.vendorID;
        gpu_info.deviceID = props.deviceID;

        gpu_info.apiVersion = QString("%1.%2.%3")
                                  .arg(VK_VERSION_MAJOR(props.apiVersion))
                                  .arg(VK_VERSION_MINOR(props.apiVersion))
                                  .arg(VK_VERSION_PATCH(props.apiVersion));


        gpu_info.driverVersion = QString("%1.%2.%3")
                                  .arg(VK_VERSION_MAJOR(props.driverVersion))
                                  .arg(VK_VERSION_MINOR(props.driverVersion))
                                  .arg(VK_VERSION_PATCH(props.driverVersion));


        for (size_t i = 0; i < memProps.memoryHeapCount; ++i)
        {
            if (memProps.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
            {
                gpu_info.vram_size += (memProps.memoryHeaps[i].size >> 20);
            }
        }

        // Присваиваем рейтинг по типу GPU
        if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            // +1000 очков Гриффендору за дискретность
            gpu_info.score += 1000;
        }
        else if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
        {
            // +500 Пуффендую - интегрированный, но за факт поддрежки вулкан
            gpu_info.score += 500;
        }
        else if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU)
        {
            // +300 Когтеврану (я не знаю что такое виртуальный GPU, но прикольно звучит)
            gpu_info.score += 300;
        }
        else
        {
            // Слизерину чтоб не выеживался сильно тоже накинем...
            gpu_info.score += 100;
        }

        // По баллу за каждый Гб VRAM
        gpu_info.score += (gpu_info.vram_size >> 10);

        // Оцениваем поддерживаемую версию Vulkan API
        uint32_t major = VK_VERSION_MAJOR(props.apiVersion);
        uint32_t minor = VK_VERSION_MINOR(props.apiVersion);

        gpu_info.score += static_cast<int>(major * 100) + static_cast<int>(minor * 10);

        gpus_info.push_back(gpu_info);

        if (coreFeat2.features.samplerAnisotropy)   gpu_info.score += 50;
        if (coreFeat2.features.geometryShader)      gpu_info.score += 30;
        if (coreFeat2.features.tessellationShader)  gpu_info.score += 30;
        if (coreFeat2.features.multiDrawIndirect)   gpu_info.score += 20;
        if (coreFeat2.features.fillModeNonSolid)    gpu_info.score += 15;
        if (coreFeat2.features.wideLines)           gpu_info.score += 15;
    }

    return GPU_STATE_READY;
}
