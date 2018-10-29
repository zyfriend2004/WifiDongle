#include "MainMenuView.h"
#include "MTCommonWidget.h"
#include "MTViewManager.h"
#include "wificmd.h"

//#include "ap_scan.h"
//#include "ap_playback.h"
//#include "../dvb_api/epg_store.h"

IMPLEMENT_GETVIEW_INSTANCE(MainmenuView)

MainmenuView::MainmenuView(): MTAppBaseView()
{
    _curMenuItem = 0;
}

MainmenuView::~MainmenuView()
{
}

bool CompareAttribute(TiXmlElement * pEleConfig, const char * attribute, const char * strValue)
{
    if (!pEleConfig || !attribute || !strValue) {
        mt_log("", MT_LOG_ERROR, "[%s][ERROR] para error!\n", __func__);
    }

    bool bRet = false;
    const char * lpszTmp = pEleConfig->Attribute(attribute);

    if (lpszTmp && !strcmp(lpszTmp, strValue)) {
        bRet = true;
    }

    return bRet;
}

void MainmenuView::OnLoadFromRSS(TiXmlElement * pViewEle, void * p1, void * p2)
{
    _menu[0]._pMenu = FindWidget("menu_channel");
    _menu[1]._pMenu = FindWidget("menu_media");
    _menu[2]._pMenu = FindWidget("menu_network");
    _menu[3]._pMenu = FindWidget("menu_install");
    _menu[4]._pMenu = FindWidget("menu_system");
    _menu[5]._pMenu = FindWidget("menu_tool");
    _menu[0]._pMenuButton = (MTWidgetButton *)FindWidget("btn_channel");
    _menu[1]._pMenuButton = (MTWidgetButton *)FindWidget("btn_media");
    _menu[2]._pMenuButton = (MTWidgetButton *)FindWidget("btn_network");
    _menu[3]._pMenuButton = (MTWidgetButton *)FindWidget("btn_install");
    _menu[4]._pMenuButton = (MTWidgetButton *)FindWidget("btn_system");
    _menu[5]._pMenuButton = (MTWidgetButton *)FindWidget("btn_tool");
    TiXmlElement * pTmp = NULL;
    MTMenuItem * pItem;

    for (int i = 0; i < MAIN_MENU_ITEMS; i++) {
        _menu[i]._pItemList = (MTWidgetListViewEx *)_menu[i]._pMenu->FindWidget(NULL, "widget_listview_ex");
        pTmp = _menu[i]._pItemList->_pXmlConfig->FirstChildElement("items");
        int flags = MTITEM_STATE_NORMAL;

        if (pTmp) {
            for (pTmp = (TiXmlElement *)pTmp->FirstChild(); pTmp != 0; pTmp = (TiXmlElement *)pTmp->NextSibling()) {
                pItem = new MTMenuItem(pTmp->Attribute("text"), pTmp->Attribute("pic"), pTmp->Attribute("link"), pTmp->Attribute("para"));

                if (CompareAttribute(pTmp, "language", "able")) {
                    pItem->_stWinTrait.SetStatus(MTWINDOWTRAIT_SHOW_LANGUAGE_STR);
                }

                flags = MTITEM_STATE_NORMAL;

                if (strlen(pTmp->Attribute("link")) == 0) {
                    flags = MTITEM_STATE_DISABLED;
                }

                _menu[i]._pItemList->AddListItem(pItem, flags);
                _menu[i]._pItemList->_DrawItemFun.bindfun(this, &MainmenuView::DrawListItem);
            }
        }
    }
}

BOOL MainmenuView::ProcessMessage(MT_MESSAGE_BUF &msg)
{
    mt_log("", MT_LOG_DEBUG, "------- [%s:%s:%d]userInput = %d \n", __FILE__, __func__, __LINE__, msg.message);
    _menu[_curMenuItem]._pItemList->ProcessMessage(msg);
    int oldItem = _curMenuItem;


    /*if (MT_USB_PLUGIN == msg.message)
    {
        mt_log("", MT_LOG_DEBUG, "[%s][%s] get msg MT_USB_PLUGIN\n", __FILE__, __func__);
        wifi_Station_Start();
		    sleep(3);
		    wifi_Station_Scan();
    }
    else if (MT_WIFI_SCAN_RESULT_AVAILABLE == msg.message)
    {
        mt_log("", MT_LOG_DEBUG, "[%s][%s] get msg MT_WIFI_SCAN_RESULT_AVAILABLE!\n", __FILE__, __func__);
    	wifi_Station_GetScanList();
		sleep(5);

		wifi_Station_Getstats();

		wifi_Station_Connect("fastfwr310","u9eQnhxq4Vi0");
    }
    else if(MT_WIFI_CONNECT_SUCCESS == msg.message)
    {
        mt_log("", MT_LOG_DEBUG, "[%s][%s] get msg MT_WIFI_CONNECT_SUCCESS\n", __FILE__, __func__);
		wifi_dhcp_cmd();

		sleep(5);
		do_Dlna_Init(NULL);
		sleep(3);
		do_Dlna_Start();
    }
    else */if (msg.message == MSG_KEY_LEFT) {
        _curMenuItem = (_curMenuItem - 1 + MAIN_MENU_ITEMS) % MAIN_MENU_ITEMS;
    } 
    else if (msg.message == MSG_KEY_RIGHT) 
    {
        _curMenuItem = (_curMenuItem + 1) % MAIN_MENU_ITEMS;
    } 
    else if (msg.message == MSG_KEY_OK) 
    {
        MTMenuItem * pTextItem = (MTMenuItem *)_menu[_curMenuItem]._pItemList->GetSelectItem();
        if (pTextItem && strlen(pTextItem->szLink) > 0) 
        {
            MTViewManager::get_instance().NavigationTo(pTextItem->szLink,pTextItem->szPara , NULL);
        }
    } 
    else if (msg.message == SELECT_MENUOPTION) {
       /* if (g_epg_freq_info.pg_num > 0) {
            MT_MESSAGE_BUF msg;
            memset(&msg, 0, sizeof(MT_MESSAGE_BUF));
            msg.message = MT_MAINMENUTRAN_TRAN_AVPLAY;
            UiMsgQueue_Put(MT_MSG_ACTION_ADD, &msg, 200);
        } else {
            mt_log("", MT_LOG_DEBUG, "------- %s %d, no dvb av play, dont transform \n", __func__, __LINE__);
        }*/
    }
   

    if (oldItem != _curMenuItem) {
        _menu[oldItem]._pMenu->ShowWidget(FALSE,FALSE);
        ((MTWidgetStatic*)_menu[oldItem]._pMenu->FindWidget("item_name","widget_static"))->Drawback();
        _menu[oldItem]._pMenuButton->SetState(MTWIDGET_STATE_NORMAL);
        _menu[_curMenuItem]._pMenu->ShowWidget(TRUE);
        _menu[_curMenuItem]._pMenuButton->SetState(MTWIDGET_STATE_FOCUS);
    }

    return MTAppBaseView::ProcessMessage(msg);
}

void MainmenuView::AfterPaint()
{
    //do some draw after frame hve drawed view,most time not needed
    mt_log("", MT_LOG_DEBUG, "------- %s %d\n", __func__, __LINE__);
}

void MainmenuView::DrawListItem(int nIndex, MTBaseListItem * pBaseListItem, MTRect rc,MTCommonWidget *pListView)
{
    //do a list view item
    CGraphics * pDraw = CGraphics::graphicsInterface();
    MTMenuItem * pTextItem = (MTMenuItem *)pBaseListItem;
    Color textColor(255, 255, 255);
    Color bgColor(28, 139, 238);

    if (IS_MASKED(pTextItem->_flags, MTITEM_STATE_FOCUSED)) {
        bgColor = Color(0xff, 0x66, 0x00);
    } else if (IS_MASKED(pTextItem->_flags, MTITEM_STATE_DISABLED)) {
        textColor  = Color(0xcc, 0xcc, 0xcc);
        bgColor = Color(0x66, 0x66, 0x66);
    }

    pDraw->fillRectangle(NULL,  bgColor, rc.l, rc.t, rc.w, rc.h);
    pDraw->drawImageWithTransparentColor(NULL, pTextItem->szItemPic, rc.l + 5 , rc.t + 6, 64, 32, true, true, Color(0, 254, 255));

    if (pTextItem->szItemText != NULL) {
        char * unsafeEjectStr = pTextItem->szItemText ;

        if (pTextItem->_stWinTrait.GetStatus(MTWINDOWTRAIT_SHOW_LANGUAGE_STR)) {
            ResourceMgr::GetInstance()->GetString(pTextItem->szItemText, &unsafeEjectStr);
        }

        pDraw->drawText(NULL, unsafeEjectStr, -1,
                        rc.l + 80, rc.t + 5, rc.w - 80 , rc.h, DSTF_LEFT | DSTF_TOP, NULL,
                        textColor,  FONT_SIZE_NORMAL);
    }
}
