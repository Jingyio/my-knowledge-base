#### 2024-07-31

##### Chores

* **all:**
  *  reorganize file locations (720ada70)
  *  reorganize file locations (facc9294)
  *  reorganize file locations (64a7c8ff)

##### Continuous Integration

* **git:**  enable commitizen and commitlint features (5da126b6)

##### New Features

* **code/bare-metal/stm32/yingde-board:**
  *  add example projects (part 2) (ba77894d)
  *  add example projects (part 1) (2d251058)
* **design/pcb/yingde:**  create a core board project based on the STM32F103C8T6 microcontroller (ee3f47ff)
* **code/bare-metal/8051/easylib:**
  *  support configuring interrupt priorities (54fcde06)
  *  support power down mode (0ec470e9)
  *  add a control macro for interrupt related code (963cd827)
  *  create an easylib project based on 8051 architecture (296fbcb5)
* **code/system/windows/app/twobcam:**
  *  create a UI project to support render preview data (74d61284)
  *  support saving videos with given file names (2400945f)
  *  support video recording (6b4a248b)
  *  support getting/setting media formats (13961185)
  *  support saving captured photos with a given name (220399b9)
  *  create a twobcam project (be284366)
* **code/system/windows/driver/template:**  create template projects for KMDF and UMDF (a87e3878)
* **code/bare-metal/x86_64/demoos:**  create a demo os project based on x86 architecture (9101496c)

##### Bug Fixes

* **code/bare-metal/8051/easylib:**  resolve the issue of improper use of timer registers (7ccf417b)
* **code/system/windows/app/twobcam:**  OS decouple (506763d4)

##### Refactors

* **code/system/windows/app/twobcam:**  utilize ISinkDevice for managing render transactions (29821cec)

