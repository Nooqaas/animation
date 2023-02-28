import os
import time

import allure
import pytest
from termcolor import colored

from flippigator.case import BaseCase

os.system("color")


@pytest.mark.nfc
class TestNfc(BaseCase):
    def test_nfc_menu_negative(self, nav):
        nav.nfc.go_into()
        assert nav.nfc.check_menu() == 0, "NFC menu list differs from reference"
        nav.go_to_main_screen()

    def test_read(self, nav):
        nav.nfc.go_into()
        nav.go_to("Read")
        nav.press_ok()
        state = nav.get_current_state()
        assert "ReadingNFC" in state, "NFC Reading failed"
        nav.go_to_main_screen()

    '''
    I think that it's time to del all handy test from this repo
    Let's make only smoke test without bench?
    def test_read_ref_card(self, nav):
        with allure.step("Go to NFC"):
            nav.nfc.go_into()
        with allure.step("Read a card"):
            nav.go_to("Read")
            nav.press_ok()
            # key = cv.waitKey(550)
            nav.update_screen()
            state = nav.get_current_state()
            state = state[0]
            start_time = time.time()
            while "ReadingNFC" in state:
                print(colored("Reading", "yellow"))
                state = nav.get_current_state()
                if time.time() - start_time > 10:
                    break
            while "ReadingCardNFC" in state:
                print(colored("ReadingCardNFC", "yellow"))
                state = nav.get_current_state()
        """
        if self.navi.getCurrentState()[0] == 'AllInOneNFCRef':
            print(colored("NFC Reading passed", 'green'))
        else:
            print(colored("NFC Reading failed", 'red'))
        """
        state = nav.get_current_state()
        while "Mf Classic User Dict" in state:
            state = nav.get_current_state()
        state = nav.get_current_state()
        while "Mf Classic Flipper Dict" in state:
            nav.press_ok()
            state = nav.get_current_state()
        state = nav.get_current_state()
        while "SkipOk" in state:
            state = nav.get_current_state()
        state = nav.get_current_state()
        assert (
            "MfClassic1KTestCard" in state
        ), "Result of reading reference card is fail"
        nav.press_right()
        menu = nav.get_menu_list()
        menu_ref = [
            "Save",
            "Emulate",
            "Detect r(down)eader",
            "Info",
        ]
        assert menu == menu_ref, "NFC card menu is wrong"
        nav.go_to("Emulate")
        nav.press_ok()
        state = nav.get_current_state()
        assert "Emulating MIFARE Classic" in state, "NFC Emulation fail"
        nav.press_back()

        nav.go_to("Info")
        nav.press_ok()
        state = nav.get_current_state()
        assert "MfClassic1KTestCardInfo" in state, "Card full info wrong"
        nav.press_back()

        nav.go_to("Save")
        nav.press_ok()
        nav.press_ok()
        nav.press_ok()
        nav.press_down()
        state = nav.get_current_state()
        assert "FileBrowserLevelUp" in state, "Can't save read NFC card"
        nav.go_to_main_screen()
    '''

    def test_detect_reader(self, nav):
        nav.nfc.go_into()
        nav.go_to("Detect R(up)eader")
        nav.press_ok()
        state = nav.get_current_state()
        assert "EmulatingDetectReader" in state, "Reader detection error"
        nav.go_to_main_screen()

    def test_saved(self, nav):
        nav.nfc.go_into()
        nav.go_to("Saved")
        nav.press_ok()
        state = nav.get_current_state()
        assert "FileBrowserLevelUp" in state, "File browser in 'Saved' was not opened"
        nav.go_to_main_screen()

    def test_extra_actions(self, nav):
        nav.nfc.go_into()
        nav.go_to("Extra Actions")
        nav.press_ok()
        menu = nav.get_menu_list()
        menu_ref = [
            "Read Specific Card Type",
            "Mifare Classic Keys",
            "Unlock NTAG",
        ]
        assert menu == menu_ref, "NFC Extra Actions list is wrong"
        nav.go_to("Mifare Classic Keys")
        nav.press_ok()
        state = nav.get_current_state()
        assert (
            "Mifare Classic Keys Pict" in state
        ), "Can't find Mifare Classic Keys dict"
        nav.press_back()

        nav.go_to("Unlock NTAG")
        nav.press_ok()
        menu = nav.get_menu_list()
        menu_ref = [
            "Auth As Ameebo",
            "Auth As Xiaomi",
            "Enter Password Manual",
        ]
        assert menu == menu_ref, "NFC Extra Actions list is wrong"
        nav.press_back()
        nav.go_to_main_screen()

    def test_add_manually(self, nav):
        nav.nfc.go_into()
        nav.go_to("Add Manually")
        nav.press_ok()
        menu = nav.get_menu_list()
        menu_ref = [
            "NFC-A 7-bytes UID",
            "NFC-A 4-bytes UID",
            "Mifare Ultralight",
            "Mifare Ultralight EV1 11",
            "Mifare Ultralight EV1 H11",
            "Mifare Ultralight EV1 21",
            "Mifare Ultralight EV1 H21",
            "NTAG203",
            "NTAG213",
            "NTAG215",
            "NTAG216",
            "NTAG I2C 1k",
            "NTAG I2C 2k",
            "NTAG I2C Plus 1k",
            "NTAG I2C Plus 2k",
            "Mifare Mini",
            "Mifare Classic 1k 4byte",
            "Mifare Classic 1k 7byte",
            "Mifare Classic 4k 4byte",
            "Mifare Classic 4k 7byte",
        ]
        assert menu == menu_ref, "NFC Add manually option list is wrong"
        nav.go_to_main_screen()

    def test_debug(self, nav):
        nav.nfc.go_into()
        nav.go_to("Debug")
        nav.press_ok()

        menu = nav.get_menu_list()
        menu_ref = [
            "NFCDebugField",
            "NFCDebugApdu",
        ]
        assert menu == menu_ref, "NFC Debug options list is wrong"
        nav.go_to("NFCDebugField")
        nav.press_ok()
        state = nav.get_current_state()
        assert "NFCDebugFieldNotice" in state, "NFC Debug Field fail"
        nav.press_back()

        nav.go_to("NFCDebugApdu")
        nav.press_ok()
        state = nav.get_current_state()
        assert "Run APDU reader" in state, "NFC Debug Apdu fail"

        nav.go_to_main_screen()
