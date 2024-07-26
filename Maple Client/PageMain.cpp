#include <wx/wx.h>
#include <wx/statline.h>
#include <wx/thread.h>
#include "JsonUtil.cpp"
#include "DownloadMC.cpp"
#include "LaunchMC.cpp"
#include <wx/thread.h>

const int DOWNLOAD_COMPLETE_EVENT = wxNewEventType();

class DownloadThread : public wxThread
{
public:
    DownloadThread(wxEvtHandler* handler) : wxThread(wxTHREAD_DETACHED), m_handler(handler) {}

protected:
    virtual ExitCode Entry() override
    {
        DownloadMinecraft();
        wxQueueEvent(m_handler, new wxThreadEvent(wxEVT_THREAD, DOWNLOAD_COMPLETE_EVENT));
        return (ExitCode)0;
    }

private:
    wxEvtHandler* m_handler;
};

class CustomButton : public wxButton
{
public:
    CustomButton(wxWindow* parent, wxWindowID id, const wxString& label,
        const wxPoint& pos, const wxSize& size)
        : wxButton(parent, id, label, pos, size, wxBORDER_NONE)
    {
        Bind(wxEVT_PAINT, &CustomButton::OnPaint, this);
    }

private:
    void OnPaint(wxPaintEvent& event)
    {
        wxPaintDC dc(this);

        // 绘制按钮背景
        dc.SetBrush(wxBrush(wxColour(16, 2, 0)));
        dc.SetPen(*wxTRANSPARENT_PEN);
        dc.DrawRectangle(GetClientRect());

        // 绘制按钮文本
        dc.SetTextForeground(wxColour(221, 2, 5));
        dc.SetFont(GetFont());
        wxString label = GetLabel();
        wxSize textSize = dc.GetTextExtent(label);
        int x = (GetSize().GetWidth() - textSize.GetWidth()) / 2;
        int y = (GetSize().GetHeight() - textSize.GetHeight()) / 2;
        dc.DrawText(label, x, y);

        // 绘制边框
        dc.SetPen(wxPen(wxColour(221, 2, 5), 2)); // 设置边框颜色和宽度
        dc.SetBrush(*wxTRANSPARENT_BRUSH);
        dc.DrawRectangle(0, 0, GetSize().GetWidth(), GetSize().GetHeight());
    }
};

class MainFrame : public wxFrame
{
public:
    MainFrame() : wxFrame(nullptr, wxID_ANY, "Maple Launcher", wxDefaultPosition, wxSize(800, 500))
    {
        SetWindowStyle(wxNO_BORDER | wxFRAME_NO_TASKBAR);

        // 创建主面板
        wxPanel* mainPanel = new wxPanel(this);

        // 创建顶部栏
        wxPanel* topBar = new wxPanel(mainPanel, wxID_ANY, wxDefaultPosition, wxSize(-1, 40));
        topBar->SetBackgroundColour(wxColour(221, 2, 5));

        wxStaticText* titleText = new wxStaticText(topBar, wxID_ANY, " Maple Launcher", wxPoint(10, 10));
        titleText->SetForegroundColour(wxColour(252, 255, 252));
        titleText->SetFont(wxFont(16, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_MAX));

        wxButton* closeButton = new wxButton(topBar, wxID_ANY, "×", wxPoint(760, 5), wxSize(30, 30));
        closeButton->SetFont(wxFont(18, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
        closeButton->SetForegroundColour(wxColour(252,255,252));
        closeButton->SetBackgroundColour(wxColour(221, 2, 5));
        closeButton->SetWindowStyleFlag(wxBORDER_NONE);

        // 创建侧边栏
        wxPanel* sidebar = new wxPanel(mainPanel, wxID_ANY, wxDefaultPosition, wxSize(266, -1));
        sidebar->SetBackgroundColour(wxColour(16, 2, 0));

        wxStaticBitmap* avatar = new wxStaticBitmap(sidebar, wxID_ANY, wxBitmap("placeholder.png", wxBITMAP_TYPE_PNG), wxPoint(101, 20), wxSize(64, 64));
        wxStaticText* username = new wxStaticText(sidebar, wxID_ANY, "MCL_vbuser", wxPoint(93, 90));
        username->SetForegroundColour(wxColour(252, 255, 252));
        CustomButton* launchButton;
        if (existMCFolder()) {
            launchButton = new CustomButton(sidebar, wxID_ANY, "启动游戏", wxPoint(33, 400), wxSize(200, 40));
        }
        else {
            launchButton = new CustomButton(sidebar, wxID_ANY, "获取游戏", wxPoint(33, 400), wxSize(200, 40));
        }
        launchButton->SetForegroundColour(wxColour(221,2,5));
        launchButton->SetBackgroundColour(wxColour(16, 2, 0)); 
        launchButton->SetWindowStyleFlag(wxBORDER_NONE);

        // 创建主内容区
        wxPanel* contentArea = new wxPanel(mainPanel, wxID_ANY, wxDefaultPosition, wxSize(534, -1));
        contentArea->SetBackgroundColour(wxColour(45, 19, 23));

        // 布局
        wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
        mainSizer->Add(topBar, 0, wxEXPAND);

        wxBoxSizer* contentSizer = new wxBoxSizer(wxHORIZONTAL);
        contentSizer->Add(sidebar, 1, wxEXPAND);
        contentSizer->Add(contentArea, 2, wxEXPAND);

        mainSizer->Add(contentSizer, 1, wxEXPAND);

        mainPanel->SetSizer(mainSizer);

        // 绑定事件
        avatar->Bind(wxEVT_LEFT_DOWN, &MainFrame::OnAvatarClicked, this);
        launchButton->Bind(wxEVT_BUTTON, &MainFrame::OnLaunchGame, this);
        closeButton->Bind(wxEVT_BUTTON, &MainFrame::OnEixt, this);

        Bind(wxEVT_THREAD, &MainFrame::OnDownloadComplete, this, DOWNLOAD_COMPLETE_EVENT);
    }

private:

    CustomButton* button;

    void OnAvatarClicked(wxMouseEvent& event)
    {
        
    }

    void OnLaunchGame(wxCommandEvent& event)
    {
        if (existMCFolder()) {
            ensureMapleSettingsExist();
            runMinecraft();
        }
        else {
            button = wxDynamicCast(event.GetEventObject(), CustomButton);
            button->Enable(false);
            button->SetLabel("获取中...");

            DownloadThread* thread = new DownloadThread(this);
            if (thread->Run() != wxTHREAD_NO_ERROR)
            {
                delete thread;
                wxLogError("Failed to create the worker thread!");
            }
        }
    }

    void OnDownloadComplete(wxThreadEvent& event)
    {
        if (button)
        {
            button->SetLabel("启动游戏");
            button->Enable(true);
        }
        ensureMapleSettingsExist();
        FixNativesForge();
    }

    void OnEixt(wxCommandEvent& event){exit(0);}
};

class MyApp : public wxApp
{
public:
    bool OnInit() override
    {
        MainFrame* frame = new MainFrame();
        frame->Show();
        return true;
    }
};

int main(int argc, char* argv[])
{
    wxApp::SetInstance(new MyApp());
    wxInitializer initializer;
    if (!initializer)
    {
        wxLogError("Failed to initialize the wxWidgets library");
        return -1;
    }

    MainFrame* frame = new MainFrame();
    frame->Show();
    return wxApp::GetInstance()->MainLoop();
}