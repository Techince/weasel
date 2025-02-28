﻿#include "stdafx.h"
#include <rime_levers_api.h>
#include "WeaselDeployer.h"
#include "resource.h"
#include "SwitcherSettingsDialog.h"
import <algorithm>;
import <set>;
import <thread>;
import WeaselUtility;
import Config;

SwitcherSettingsDialog::SwitcherSettingsDialog(RimeSwitcherSettings* settings, bool& selected)
	: settings_(settings), loaded_(false), modified_(false), m_selected{ selected }
{
	api_ = (RimeLeversApi*)rime_get_api()->find_module("levers")->get_api();
}


SwitcherSettingsDialog::~SwitcherSettingsDialog()
{
	g_hwnd = nullptr;
}

void SwitcherSettingsDialog::Populate() {
	if (!settings_) return;
	RimeSchemaList available = {0};
	api_->get_available_schema_list(settings_, &available);
	RimeSchemaList selected = {0};
	api_->get_selected_schema_list(settings_, &selected);
	schema_list_.DeleteAllItems();
	int k = 0;
	std::set<RimeSchemaInfo*> recruited;
	for (int i = 0; i < selected.size; ++i) {
		const char* schema_id = selected.list[i].schema_id;
		for (int j = 0; j < available.size; ++j) {
			RimeSchemaListItem& item(available.list[j]);
			RimeSchemaInfo* info = (RimeSchemaInfo*)item.reserved;
			if (!strcmp(item.schema_id, schema_id) && recruited.find(info) == recruited.end()) {
				recruited.insert(info);
				schema_list_.AddItem(k, 0, to_wstring(item.name, CP_UTF8).data());
				schema_list_.SetItemData(k, (DWORD_PTR)info);
				schema_list_.SetCheckState(k, TRUE);
				++k;
				break;
			}
		}
	}
	for (int i = 0; i < available.size; ++i) {
		RimeSchemaListItem& item(available.list[i]);
		RimeSchemaInfo* info = (RimeSchemaInfo*)item.reserved;
		if (recruited.find(info) == recruited.end()) {
			recruited.insert(info);
			schema_list_.AddItem(k, 0, to_wstring(item.name, CP_UTF8).data());
			schema_list_.SetItemData(k, (DWORD_PTR)info);
			++k;
		}
	}

	hotkeys_.SetWindowTextW(to_wstring(api_->get_hotkeys(settings_), CP_UTF8).data());
	loaded_ = true;
	modified_ = false;
}

void SwitcherSettingsDialog::ShowDetails(RimeSchemaInfo* info) {
	if (!info) return;
	std::string details;
	if (const char* name = api_->get_schema_name(info)) {
		details += name;
	}
    if (const char* author = api_->get_schema_author(info)) {
        (details += "\n\n") += author;
    }
    if (const char* description = api_->get_schema_description(info)) {
        (details += "\n\n") += description;
    }
	description_.SetWindowTextW(to_wstring(details, CP_UTF8).data());
}

void SwitcherSettingsDialog::CloseDialog(int nVal)
{
	DestroyWindow();
	::PostQuitMessage(nVal);
}

void SwitcherSettingsDialog::GetSchemata(HWND hWndCtl)
{
	SetEnvironmentVariable(L"rime_dir", WeaselUserDataPath().data());
	HKEY hKey;
	LSTATUS ret = RegOpenKey(HKEY_LOCAL_MACHINE, LR"(Software\Rime\Weasel)", &hKey);
	if (ret == ERROR_SUCCESS) {
		WCHAR value[MAX_PATH];
		DWORD len = sizeof(value);
		DWORD type = 0;
		DWORD data = 0;
		ret = RegQueryValueExW(hKey, L"WeaselRoot", NULL, &type, (LPBYTE)value, &len);
		if (ret == ERROR_SUCCESS && type == REG_SZ) {
			WCHAR parameters[MAX_PATH + 37];
			wcscpy_s<_countof(parameters)>(parameters, (std::format(LR"(/k "{}\rime-install.bat")", value).data()));
			SHELLEXECUTEINFOW cmd = {
				sizeof(SHELLEXECUTEINFO),
				SEE_MASK_NOCLOSEPROCESS | SEE_MASK_NOASYNC,
				hWndCtl,
				L"open",
				L"cmd",
				parameters,
				NULL,
				SW_SHOW,
				NULL, NULL, NULL, NULL, NULL, NULL, NULL
			};
			ShellExecuteExW(&cmd);
			WaitForSingleObject(cmd.hProcess, INFINITE);
			CloseHandle(cmd.hProcess);
			api_->load_settings(reinterpret_cast<RimeCustomSettings*>(settings_));
			Populate();
		}
	}
	RegCloseKey(hKey);
}

LRESULT SwitcherSettingsDialog::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
{
	schema_list_.SubclassWindow(GetDlgItem(IDC_SCHEMA_LIST));
	schema_list_.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
	schema_list_.AddColumn(L"方案名称", 0);
	CRect rc;
	schema_list_.GetClientRect(&rc);
	schema_list_.SetColumnWidth(0, rc.Width() - 20);

	description_.Attach(GetDlgItem(IDC_SCHEMA_DESCRIPTION));
	
	hotkeys_.Attach(GetDlgItem(IDC_HOTKEYS));
	hotkeys_.EnableWindow(FALSE);

	get_schemata_.Attach(GetDlgItem(IDC_GET_SCHEMATA));
	get_schemata_.EnableWindow(TRUE);	
	
	Populate();

	ShowDetails((RimeSchemaInfo*)(schema_list_.GetItemData(0)));
	
	CenterWindow();
	BringWindowToTop();
	g_hwnd = m_hWnd;
	return TRUE;
}

LRESULT SwitcherSettingsDialog::OnClose(UINT, WPARAM, LPARAM, BOOL&) 
{
	CloseDialog(0);
	return 0;
}

LRESULT SwitcherSettingsDialog::OnGetSchemata(WORD, WORD, HWND hWndCtl, BOOL&)
{
	std::thread t{ &SwitcherSettingsDialog::GetSchemata, this, hWndCtl };
	t.detach();
	return 0;
}

LRESULT SwitcherSettingsDialog::OnOK(WORD, WORD code, HWND, BOOL&) 
{
	if (modified_ && settings_ && schema_list_.GetItemCount() != 0) {
		const char** selection = new const char*[schema_list_.GetItemCount()];
		int count = 0;
		for (int i = 0; i < schema_list_.GetItemCount(); ++i) {
			if (!schema_list_.GetCheckState(i))
				continue;
			RimeSchemaInfo* info = (RimeSchemaInfo*)(schema_list_.GetItemData(i));
			if (info) {
				selection[count++] = api_->get_schema_id(info);
			}
		}
		if (count == 0) {
			// MessageBox(L"至少要选用一项吧。", L"小狼毫不是这般用法", MB_OK | MB_ICONEXCLAMATION);
			delete selection;
			m_selected = false;
			return false;
		}
		api_->select_schemas(settings_, selection, count);
		delete selection;
	}
	CloseDialog(0);
	return 0;
}

LRESULT SwitcherSettingsDialog::OnSchemaListItemChanged(int, LPNMHDR p, BOOL&) {
	LPNMLISTVIEW lv = reinterpret_cast<LPNMLISTVIEW>(p);
	if (!loaded_ || !lv || lv->iItem < 0 && lv->iItem >= schema_list_.GetItemCount())
		return 0;
	if ((lv->uNewState & LVIS_STATEIMAGEMASK) != (lv->uOldState & LVIS_STATEIMAGEMASK)) {
		modified_ = true;
	}
	else if ((lv->uNewState & LVIS_SELECTED) && !(lv->uOldState & LVIS_SELECTED)) {
		ShowDetails((RimeSchemaInfo*)(schema_list_.GetItemData(lv->iItem)));
	}
	return 0;
}
