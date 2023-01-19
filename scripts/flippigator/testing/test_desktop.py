import os

import pytest

from flippigator.case import BaseCase

os.system("color")


@pytest.mark.menu
class TestMain(BaseCase):
    """
    Here lies the test cases for the menu
    """

    def test_skipped(self, nav):
        """
        This test is skipped
        """
        nav.go_to_main_screen()
        assert False, "This test is skipped"

    def test_skipped2(self, nav):
        """
        This test is skipped
        """
        nav.go_to_main_screen()
        nav.press_ok()
        nav.update_screen()
        assert False, "This test is skipped"

    def test_main_screen_negative(self, nav):  # can have multiple fixtures and args
        """ "
        Negative test case for main screen
        """
        nav.go_to_main_screen()
        state = nav.get_current_state()
        assert "SDcardIcon" in state and "BTIcon" in state, "Can't reach main screen"

    def test_menu_option_list_negative(self, nav):
        """
        Check that all menu options are present
        """
        nav.go_to_main_screen()
        nav.press_ok()
        assert nav.get_menu_list() == [
            "Sub-GHz",
            "RFID",
            "NFC",
            "Infrared",
            "GPIO",
            "iButton",
            "BadUSB",
            "U2F",
            "Apps",
            "Settings",
        ]