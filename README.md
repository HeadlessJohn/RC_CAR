# RC Car Project
## Nucleo F411RE 보드를 이용한 RC Car
### 1. 기능
- 블루투스로 조작
- 전진, 후진, 좌회전, 우회전
- 전진, 좌회전, 우회전 속도 제어
- LCD에 상태 표시
### 2. 사용한 부품
- `Nucleo F411RE STM32`보드
- `HC-06` 블루투스 모듈
- `L298N` 모터 드라이버
- DC 모터 4개
- 18650 3.6V 배터리 x3
### 3. OS, API
- FreeRTOS CMSIS-RTOS V2
- ssd1306 library