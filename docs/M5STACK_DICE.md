# M5Stack 搖動擲骰：學生整合任務

目前遊戲還沒有 M5Stack 連線功能。這份文件是開發計畫，不表示硬體已測試完成。

## 目標與流程

1. 確認實際 M5Stack 型號是否內建 IMU，或需外接加速度感測器；安裝對應開發環境與函式庫。
2. 定期讀取三軸加速度，偵測明顯的搖動。設定閾值、消除重複觸發，靜止後才允許下一次搖動；閾值與時間應以實機調校。
3. 裝置透過選定的連線方式傳出「搖動」事件，由使用者操作的遊戲客戶端轉成擲骰請求。
4. 只有自己的回合、處於可擲骰狀態且連線正常時才送出一次 `roll`；其他狀態顯示原因。骰子數仍由交通工具決定，點數由遊戲規則產生。
5. 斷線時保留原有按鈕；重新連線後不重送舊搖動事件。

## 建議開發順序

- **第一階段：感測器**：先在序列埠顯示加速度並確認實際型號與座標、單位；例如支援 M5Unified 的機種可參考[官方 IMU 文件](https://docs.m5stack.com/en/arduino/m5unified/imu_class)與 `M5.Imu.getImuData()`。不同型號的 API 與 IMU 可能不同。
- **第二階段：傳輸**：先選擇實驗環境。電腦端可評估 USB Web Serial 或 BLE Web Bluetooth，兩者均受瀏覽器與 HTTPS/localhost 限制；手機尤其需要先確認支援狀況。也可設計同網路的配對中介服務，但需要自行實作身分驗證與安全傳輸。參考 [Web Serial](https://developer.mozilla.org/en-US/docs/Web/API/Web_Serial_API)、[Web Bluetooth](https://developer.mozilla.org/en-US/docs/Web/API/Web_Bluetooth_API)。
- **第三階段：串接遊戲**：先找 `src/main.ts` 的 `canAct()`、`act()`、`#turn-action` 與 `src/engine.ts` 的 `roll` 動作。讓硬體觸發共用同一合法擲骰流程，勿用裝置直接決定或覆寫骰點，也不要讓搖動在抵達後誤觸「結束回合」。
- **第四階段：多人驗證**：多人走 `src/network.ts` 的 `RoomClient.command()`，房間服務驗證玩家身分、回合與狀態；多人骰點由伺服器規則決定。裝置應配對到正確玩家，而非讓所有玩家的裝置都能操作同一個席位。

## 驗收清單

- 一次有效搖動只擲一次；持續晃動、背景噪聲與斷線重連不重複觸發。
- 非本人回合、移動動畫中、等待選項或遊戲結束時不擲骰，也不自動結束回合。
- 步行、機車／腳踏車、汽車分別使用遊戲既有的骰子數；多人狀態在所有玩家畫面一致。
- 裝置斷線仍可使用螢幕按鈕；重新配對後可繼續遊玩。
- 測試至少包含感測器閾值與去彈跳、回合權限、重複訊息、離線及多人流程，再用實際 M5Stack、目標手機及瀏覽器驗證。

不要把房間憑證、Wi-Fi 密碼或 API 金鑰寫入公開程式碼。若未來使用網路中介，應加入裝置配對、驗證、加密與速率限制；在教學環境先以本機或隔離網路驗證。


## 本專案提供的 M5StickC Plus USB 原型實作

本專案依照 M5StickC Plus（ESP32-PICO-D4、內建 MPU6886 六軸 IMU）設定；不適用於 M5Stack Basic/Core。感測器讀取與搖動門檻仍須以實機調校。

- `firmware/`：使用 M5Unified 讀取內建 IMU，加速度變化達門檻後輸出單行 `SHAKED`。搖動後需先安靜一段時間才重新 armed，避免連續觸發。
- `platformio.ini`：PlatformIO 目標為 `m5stick-c`，環境名稱 `m5stickc-plus`。
- `.github/workflows/build-m5stack.yml`：GitHub Actions 自動編譯並上傳韌體檔案，不需要 Arduino IDE。編譯產物包含 `firmware.bin`、`bootloader.bin`、`partitions.bin`、`boot_app0.bin`。
- 遊戲右下角「連接 M5StickC Plus（USB）」使用 Web Serial 接收訊息；瀏覽器需使用最新版 Chrome 或 Edge，且遊戲必須從 HTTPS 或 localhost 開啟。
- 收到訊號後，遊戲僅在 `canAct()` 成立且 `state.phase === 'roll'` 時呼叫既有 `act({ type: 'roll' })`。骰點仍由遊戲原本規則產生；原有畫面擲骰按鈕保留，USB 斷線不會停用按鈕。

### 免 Arduino IDE 編譯與燒錄

1. 將專案推送到自己的 GitHub repository。
2. 開啟 repository 的 **Actions**，執行 **Build M5StickC Plus dice firmware**（或推送相關檔案自動觸發）。
3. 在成功的 workflow run 下載 `m5stickc-plus-firmware` artifact 並解壓縮。
4. 使用桌面版 Chrome／Edge 開啟 [ESP Web Tool](https://esptool.spacehuhn.com/)，連接 M5StickC Plus 的 USB，選擇對應 COM 埠。
5. 加入下列四個檔案並確認燒錄位址：
   - `bootloader.bin` → `0x1000`
   - `partitions.bin` → `0x8000`
   - `boot_app0.bin` → `0xE000`
   - `firmware.bin` → `0x10000`
6. 執行燒錄。不要選擇「Erase entire flash」，除非有明確需要清除整顆 Flash。
7. 重開後螢幕應顯示 `Labu Dice Ready`；搖動時序列埠會輸出 `SHAKED`。
8. 開啟大富翁遊戲，按「連接 M5StickC Plus（USB）」並選擇裝置的 COM 埠。每位多人遊戲玩家需在自己的瀏覽器連接自己的裝置。

注意：此版本是待實機驗證的初版，不能宣稱已經在使用者的硬體上測試成功。若 Windows 沒有出現 COM 埠，需先安裝 M5Stack 官方頁面提供的 USB／FTDI 驅動。Web Serial 只支援 Chromium 系瀏覽器；瀏覽器不支援或裝置斷線時，原本的畫面擲骰按鈕仍可使用。

官方硬體規格與 USB 驅動說明：[M5StickC Plus](https://docs.m5stack.com/en/core/StickC-Plus)。
