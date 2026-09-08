#include "CharCode.h"
#include "Dxlib.h"

namespace file_util {
    bool HasUtf8Bom(const char* filepath) {
        std::ifstream file(filepath, std::ios::binary);
        if (!file) return false;

        unsigned char bom[3] = { 0 };
        file.read(reinterpret_cast<char*>(bom), 3);

        // UTF-8のBOMは、EF BB BF という3バイトの並び
        return (bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF);
    }

    void SetFileCharCode(const char* filepath, int handle) {

        // BOMの有無で、文字コードを判定して設定する
        if (HasUtf8Bom(filepath)) {
            FileRead_set_format(DX_CHARCODEFORMAT_UTF8, handle);
        }
        else {
//            FileRead_set_format(DX_CHARCODEFORMAT_SHIFTJIS, handle); // BOMが無ければShift-JISとみなす
            FileRead_set_format(DX_CHARCODEFORMAT_UTF8, handle);
        }

    }

}