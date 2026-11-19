# M5StickC Plus 搖動擲骰整合版

本版本目標硬體為 **M5StickC Plus（第一代，ESP32-PICO-D4，內建 MPU6886）**，不是 M5Stack Basic，也不是 M5StickC Plus2。

## 免 Arduino IDE 流程

1. 將此專案上傳至自己的 GitHub repository。
2. 到 GitHub repository 的 **Actions**，執行 **Build M5StickC Plus dice firmware**。
3. 成功後下載 workflow 的 `m5stickc-plus-firmware` artifact 並解壓縮。
4. 在 Windows 使用最新版 Chrome 或 Edge 開啟 https://esptool.spacehuhn.com/。
5. 按 Connect，選擇 M5StickC Plus 的 COM 埠。
6. 加入 artifact 內的四個 `.bin`，確認位址：
   - `bootloader.bin`：`0x1000`
   - `partitions.bin`：`0x8000`
   - `boot_app0.bin`：`0xE000`
   - `firmware.bin`：`0x10000`
7. 按 Program 燒錄。不要勾選 Erase entire flash，除非你確定要清除整顆 Flash。
8. 裝置重啟後顯示 `Labu Dice Ready` 即代表韌體啟動；搖動時序列埠輸出 `SHAKED`。
9. 開啟部署在 HTTPS 網址或本機 localhost 的遊戲，按右下角 **連接 M5StickC Plus（USB）**，選擇同一個 COM 埠。

## 遊戲端

- 只在遊戲已開始、目前玩家可操作、遊戲階段為 `roll` 且不是 NPC 回合時接受搖動。
- 一次搖動透過韌體的靜止重新觸發機制及遊戲端冷卻時間防止重複擲骰。
- 搖動只呼叫原有 `act({ type: 'roll' })`，骰點與骰子數仍由遊戲規則決定。
- 畫面擲骰按鈕保留；USB 斷線時仍可使用。

## 注意

此為待實機驗證版本。GitHub Actions 編譯成功不等於已在實際 M5StickC Plus 上完成燒錄與感測測試。Web Serial 需要桌面版 Chrome／Edge；Firefox、Safari 不支援。若 Windows 沒有顯示 COM 埠，需依 M5Stack 官方 M5StickC Plus 文件安裝相應 USB 驅動：https://docs.m5stack.com/en/core/StickC-Plus。
