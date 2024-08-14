# ShellcodeLoader
<a><img alt="security" src="https://img.shields.io/badge/security-audited-green?style=flat-square%22%20alt=%22Security"></a>
<a><img alt="Remote Containers" src="https://img.shields.io/badge/Remote%20Containers-Compatible-green.svg"></a>

SLoader 是一個幫助遠端控制工具繞過防毒軟體檢測的工具。

在進行深入攻防滲透時，常常需要植入 Cobalt Strike、Metasploit Framework、sliver 等工具的木馬程序來維持權限，但會被防毒軟體檢測到。Loader 可以協助遠端控制工具繞過 Defender、趨勢科技、Norton、McAfee 等防毒軟體的檢測。

## 技術實現

自訂擴展：利用模板自訂加載方式，創造新的加載方式。

隨機哈希：隨機生成檔案名稱，並以時間戳作為鍵對 URL 進行加密。

隱秘連結：loader 遠端加密連結 payload。

多重加密：使用 RC4、Base64、AES 演算法進行 payload 的加解密。

## 編譯環境

編譯環境：生成器、編碼器與加載模板均使用 C++ 開發，通過 Visual Studio 2022 靜態編譯。

編譯方式：通過 Visual Studio 2022 打開解決方案 (*.sln)，選擇 release、x64 進行編譯。或直接下載使用[realse](https://github.com/ED-E92/SLoader/releases/tag/SLoader)。

## 使用方式

使用 Cobalt Strike 生成 raw 格式的 shellcode 檔案（shellcode 的位元數取決於加載器模板的位元數）

![image](https://github.com/user-attachments/assets/e264878a-a7a8-4c51-92c3-eff5415d79d4)


通過 encode 對 shellcode 進行加密（加密金鑰需要 16 位元組）

![image](https://github.com/user-attachments/assets/21089a27-b123-4cc0-8960-96a6ce8ed3d5)

將生成的 bmp 檔案上傳至遠端 WEB 伺服器生成下載連結（訪問 URL 可直接下載檔案）

![image](https://github.com/user-attachments/assets/00ca4abe-d67a-4abc-80e0-fd379bbc2c93)

打開生成器，填寫 bmp 檔案的 URL 與加密金鑰，選擇自訂的加載器模板（DATA 目錄）

![image](https://github.com/user-attachments/assets/3e290a4a-7c27-405c-8ee6-fc0a03e0b9d1)

點選 Generate，桌面上會生成可執行的加載器檔案

![image](https://github.com/user-attachments/assets/e51e9ce9-bd05-45ec-aa9f-228fd081e587)


## 實際效果

### Norton （Norton 會攔截沒有簽名的程式上網，因此需要對程式進行簽名）

![image](https://github.com/user-attachments/assets/720df57d-6b63-40bd-a6f4-fe598a68e85a)

### Defender

![image](https://github.com/user-attachments/assets/dfebdf5e-c484-427c-bce7-7e4bc9e70a54)

### Trend Micro (趨勢科技)

![image](https://github.com/user-attachments/assets/03bdecb3-077e-4f0b-b2fc-6ff04add8f33)

## 自訂擴展

打開 plug 資料夾下的模板檔案。

![image](https://github.com/user-attachments/assets/aac91ab2-42a3-4fb7-9925-2ea21785ef3c)

自訂 testIaT_inj.cpp 代碼內容，選擇自己的方式來加載 shellcode。

![image](https://github.com/user-attachments/assets/3b6e9375-cc54-4f6c-9b97-a69fe76cfb95)

根據代碼內容與 shellcode 選擇 64 位或 32 位進行靜態編譯，將編譯出的檔案進行命名，放入 DATA 目錄中，這將在 UI 上顯示為加載方式名稱。

![image](https://github.com/user-attachments/assets/aeb79411-0771-46a8-887b-7491093ca857)
