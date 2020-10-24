#include "appskin.h"
#include "EmWinHZFont.h"//teng

void setFramewinCustomSkin(int custom) {
    FRAMEWIN_SKINFLEX_PROPS props;
    
    if (custom == 0) {
        // 不使用暗黑模式
        FRAMEWIN_GetSkinFlexProps(&props, FRAMEWIN_SKINFLEX_PI_ACTIVE);
        props.Radius = 0;
        FRAMEWIN_SetSkinFlexProps(&props, FRAMEWIN_SKINFLEX_PI_ACTIVE);
        FRAMEWIN_GetSkinFlexProps(&props, FRAMEWIN_SKINFLEX_PI_INACTIVE);
        props.Radius = 0;
        FRAMEWIN_SetSkinFlexProps(&props, FRAMEWIN_SKINFLEX_PI_INACTIVE);
        return;
    }
    // 使用暗黑模式
    FRAMEWIN_GetSkinFlexProps(&props, FRAMEWIN_SKINFLEX_PI_ACTIVE);
    props.aColorFrame[0] = 0x000A0A0A;
    props.aColorFrame[1] = 0x00333333;
    props.aColorFrame[2] = 0x00292929;
    props.aColorTitle[0] = 0x00333333;
    props.aColorTitle[1] = 0x00222222;
    props.Radius = 0;
    FRAMEWIN_SetSkinFlexProps(&props, FRAMEWIN_SKINFLEX_PI_ACTIVE);
    FRAMEWIN_GetSkinFlexProps(&props, FRAMEWIN_SKINFLEX_PI_INACTIVE);
    props.aColorFrame[0] = 0x000A0A0A;
    props.aColorFrame[1] = 0x00333333;
    props.aColorFrame[2] = 0x00292929;
    props.aColorTitle[0] = 0x00333333;
    props.aColorTitle[1] = 0x00222222;
    props.Radius = 0;
    FRAMEWIN_SetSkinFlexProps(&props, FRAMEWIN_SKINFLEX_PI_INACTIVE);
    
    FRAMEWIN_SetDefaultClientColor(0x00333333);// 客户区颜色
    FRAMEWIN_SetDefaultTextColor(0, 0x00CCCCCC);// 0 未激活
    FRAMEWIN_SetDefaultTextColor(1, 0x00EEEEEE);// 1 激活
}

void setButtonCustomSkin(int custom) {
    BUTTON_SKINFLEX_PROPS props;
    if (custom == 0) {
        
        return;
    }
    BUTTON_GetSkinFlexProps(&props, BUTTON_SKINFLEX_PI_PRESSED);
    props.aColorUpper[0] = 0x00AAAAAA;
    props.aColorUpper[1] = 0x00A0A0A0;
    props.aColorLower[0] = 0x00999999;
    props.aColorLower[1] = 0x00909090;
    props.aColorFrame[2] = 0x009D9D9D;
    BUTTON_SetSkinFlexProps(&props, BUTTON_SKINFLEX_PI_PRESSED);
    
    BUTTON_GetSkinFlexProps(&props, BUTTON_SKINFLEX_PI_FOCUSSED);
    props.aColorUpper[0] = 0x00AAAAAA;
    props.aColorUpper[1] = 0x00A0A0A0;
    props.aColorLower[0] = 0x00999999;
    props.aColorLower[1] = 0x00909090;
    props.aColorFrame[2] = 0x009D9D9D;
    BUTTON_SetSkinFlexProps(&props, BUTTON_SKINFLEX_PI_FOCUSSED);
    
    BUTTON_GetSkinFlexProps(&props, BUTTON_SKINFLEX_PI_ENABLED);
    props.aColorUpper[0] = 0x00AAAAAA;
    props.aColorUpper[1] = 0x00A0A0A0;
    props.aColorLower[0] = 0x00999999;
    props.aColorLower[1] = 0x00909090;
    props.aColorFrame[2] = 0x009D9D9D;
    BUTTON_SetSkinFlexProps(&props, BUTTON_SKINFLEX_PI_ENABLED);
    
    BUTTON_GetSkinFlexProps(&props, BUTTON_SKINFLEX_PI_DISABLED);
    props.aColorUpper[0] = 0x00AAAAAA;
    props.aColorUpper[1] = 0x00A0A0A0;
    props.aColorLower[0] = 0x00999999;
    props.aColorLower[1] = 0x00909090;
    props.aColorFrame[2] = 0x009D9D9D;
    BUTTON_SetSkinFlexProps(&props, BUTTON_SKINFLEX_PI_DISABLED);
    
    BUTTON_SetDefaultTextColor(0x00EEEEEE, BUTTON_BI_DISABLED);
    BUTTON_SetDefaultTextColor(0x00EEEEEE, BUTTON_BI_PRESSED);
    BUTTON_SetDefaultTextColor(0x00EEEEEE, BUTTON_BI_UNPRESSED);
}

void setAppCustomSkin(int custom) {
    setFramewinCustomSkin(custom);
    setButtonCustomSkin(custom);
    

//    SWIPELIST_SetDefaultFont(SWIPELIST_FI_ITEM_HEADER, &GUI_FontHZ16);
//    SWIPELIST_SetDefaultFont(SWIPELIST_FI_ITEM_TEXT, &GUI_FontHZ12);
//    SWIPELIST_SetDefaultFont(SWIPELIST_FI_SEP_ITEM, &GUI_FontHZ16);
//    
//    SWIPELIST_SetDefaultBkColor(SWIPELIST_CI_BK_ITEM_SEL, GUI_GRAY);
//    SWIPELIST_SetDefaultBkColor(SWIPELIST_CI_BK_ITEM_UNSEL, GUI_LIGHTGRAY);
//    SWIPELIST_SetDefaultBkColor(SWIPELIST_CI_BK_SEP_ITEM, GUI_GRAY_AA);
//    
//    
//    SWIPELIST_SetDefaultTextColor(SWIPELIST_CI_ITEM_HEADER_UNSEL, GUI_BLACK);
//    SWIPELIST_SetDefaultTextColor(SWIPELIST_CI_ITEM_HEADER_SEL, GUI_DARKGRAY);
//    SWIPELIST_SetDefaultTextColor(SWIPELIST_CI_ITEM_TEXT_UNSEL, GUI_DARKGRAY);
//    SWIPELIST_SetDefaultTextColor(SWIPELIST_CI_ITEM_TEXT_SEL, GUI_DARKGRAY);
//    SWIPELIST_SetDefaultTextColor(SWIPELIST_CI_SEP_ITEM_TEXT, GUI_DARKGRAY);
}



