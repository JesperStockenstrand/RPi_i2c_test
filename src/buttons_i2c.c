/* RPi_i2c_test - Used together with schematics at http://raspify.stockenstrand.com  
 * Copyright (C) 2013 Jesper Stockenstrand
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.	
 */

#include <linux/input.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>

int btn;
int fd;

char *BUTTONS = "/dev/i2c-0";
int BTNaddress = 0x24;
int buttonsConnected = -1; // -1 = unknown, 0 = not connected, 1 = connected
int preButton = 0;
void initButtons() {
    if ((btn = open(BUTTONS, O_RDWR)) < 0) {
        printf("Failed to open the i2c bus\n");
        buttonsConnected = 0;
        return;
    }
    if (ioctl(btn, I2C_SLAVE, BTNaddress) < 0) {
        printf("Failed to acquire bus access and/or talk to slave.\n");
        buttonsConnected = 0;
        return;
    }
    buttonsConnected = 1;
}

int checkButton() {
    char buf[1];
    int button = 0;

    if (buttonsConnected == -1) {
        initButtons();
    }
    if (buttonsConnected == 0) {
        return 999;
    }


if (buttonsConnected == 1) {
    if (read(btn, buf, 1) != 1) {
        printf("Error reading from i2c\n");
        buttonsConnected = 0;
    }
    else {
      switch(buf[0]) {
        case 127:
          if (preButton != 1) {
            preButton = 1;
            button = 1;
          }
          break;

        case 191:
          if (preButton != 2) {
            preButton = 2;
            button = 2;
          }
          break;

        case 223:
          if (preButton != 3) {
            preButton = 3;
            button = 3;
          }
          break;

        case 239:
          if (preButton != 4) {
            preButton = 4;
            button = 4;
          }
          break;

        default:
          preButton = 0;

      }
    }
  }
  return button;
  
}
