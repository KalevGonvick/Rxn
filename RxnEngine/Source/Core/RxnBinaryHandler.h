#pragma once
#include <list>

namespace Rxn::Core
{
    const uint8_t RXN_BUNDLE_HEADER_START [6] = { 0x01, 0x52, 0x58, 0x4E, 0x42, 0x42 };  // 6 * 1 bytes (6)
    const uint8_t RXN_BUNDLE_HEADER_END   [6] = { 0x01, 0x52, 0x58, 0x4E, 0x42, 0x42 };  // 6 * 1 bytes (6)

    const uint8_t RXN_FILE_HEADER_START   [6] = { 0x01, 0x52, 0x58, 0x4E, 0x00, 0x00 };  // 6 * 1 bytes (6)
    const uint8_t RXN_FILE_HEADER_END     [6] = { 0x01, 0x52, 0x58, 0x4E, 0x00, 0x00 };  // 6 * 1 bytes (6)

    enum class FileCategory
    {
        UNDEFINED,
        CONFIG,
        ASSETS
    };

    struct RXN_BASE_FILE
    {
        FileCategory fileCategory;
    };

    struct RXN_BUNDLE_HEADER_STRUCT 
    {
        uint8_t  fileHeader[6];           //   6 * 1 bytes   (6)
        uint32_t subFileCount;            //   1 * 4 bytes   (4)
        uint32_t epoch_pack_time;         //   1 * 4 byte    (4)
        uint8_t  file_name[128];          // 128 * 1 bytes (128)
        uint8_t  engine_version[128];     // 128 * 1 bytes (128)
    };                                    //         bytes (270)

    struct RXN_FILE_HEADER_STRUCT
    {
        uint8_t  fileHeader[6];           //   6 * 1 bytes   (6)
        uint32_t epoch_pack_time;         //   1 * 4 byte    (4)
        uint8_t  file_name[128];          // 128 * 1 bytes (128)
        uint8_t  engine_version[128];     // 128 * 1 bytes (128)
    };                                    //         bytes (266)

    struct RXN_CONFIG_STRUCT : RXN_BASE_FILE
    {

    };

    struct RXN_ASSET_STRUCT : RXN_BASE_FILE
    {
    
    };

    // used for packing binary data (assets, sounds, etc.)
    // can also be used to make internal configs for devs
    // .rxnb  - rxn binary
    // .rxnbb - rxn bundle binary
    class RXN_ENGINE_API RxnBinaryHandler
    {
    private:
        
        static RxnBinaryHandler * INSTANCE;
        static RxnBinaryHandler * Instance() { return INSTANCE; }

    public:
        
        RxnBinaryHandler();
        ~RxnBinaryHandler();

    public:
        
        static void ReadRxnFile(const wchar_t * fileName, bool decrypt = true);
        static void WriteRxnFile(const wchar_t* fileName, const std::list<RXN_BASE_FILE> content, bool encrypt = true);
        static void ReadRxnEngineConfigFile();

    private:
        
        static bool   WriteRxnBundle(const wchar_t * fileName, std::list<RXN_BASE_FILE> content);
        static bool   WriteRxnSingleFile(const wchar_t* fileName, const RXN_BASE_FILE &content);
        static bool   EncryptRxnFile(const wchar_t* fileName);
        static bool   DecryptRxnFile(const wchar_t* fileName);
        static double Random();
        
        double m_dRandomResult;
    };
}