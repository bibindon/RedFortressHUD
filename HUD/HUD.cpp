#include "HUD.h"
#include <sstream>
#include <algorithm>

using namespace NSHUD;

static std::vector<std::wstring> split(const std::wstring& s, wchar_t delim)
{
    std::vector<std::wstring> result;
    std::wstringstream ss(s);
    std::wstring item;

    while (getline(ss, item, delim))
    {
        result.push_back(item);
    }

    return result;
}

void HUD::Init(IFont* font, ISprite* sprBack, ISprite* sprMiddle, ISprite* sprFront, const bool bEnglish)
{
    m_font = font;
    m_sprBack = sprBack;
    m_sprMiddle = sprMiddle;
    m_sprFront = sprFront;
    m_bEnglish = bEnglish;

    m_font->Init(m_bEnglish);
}

void NSHUD::HUD::Finalize()
{
    delete m_font;
    delete m_sprBack;
    delete m_sprMiddle;
    delete m_sprFront;
}

void NSHUD::HUD::UpsertStatus(const std::wstring& name,
                                        const int percent,
                                        const int percentSub,
                                        const bool visible)
{
    auto result = std::find_if(m_statusList.begin(), m_statusList.end(),
                               [&](const StatusItem& x)
                               {
                                   return x.GetName() == name;
                               });

    if (result != m_statusList.end())
    {
        result->SetPercent(percent);
        result->SetPercentSub(percentSub);
        result->SetBarVisible(visible);
    }
    else
    {
        StatusItem statusItem;
        statusItem.SetName(name);
        statusItem.SetPercent(percent);
        statusItem.SetPercentSub(percentSub);
        statusItem.SetBarVisible(visible);

        m_statusList.push_back(statusItem);
    }
}

void NSHUD::HUD::RemoveStatus(const std::wstring& name)
{
    auto result = std::remove_if(m_statusList.begin(), m_statusList.end(),
                                 [&](const StatusItem& x)
                                 {
                                     return x.GetName() == name;
                                 });

    m_statusList.erase(result, m_statusList.end());
}

void HUD::Draw()
{
    // どれだけステータス異常があっても表示できるのは8行までとする？
    for (size_t i = 0; i < 8; ++i)
    {
        if (m_statusList.size() <= i)
        {
            break;
        }

        m_font->DrawText_(m_statusList.at(i).GetName(),
                          STARTX + 10,
                          STARTY + (INTERVAL * (int)i));

        if (m_statusList.at(i).GetBarVisible())
        {
            m_sprBack->DrawImage(100,
                                 STARTX,
                                 PADDING + STARTY + (INTERVAL * (int)i));

            m_sprMiddle->DrawImage(m_statusList.at(i).GetPercentSub(),
                                   STARTX,
                                   PADDING + STARTY + (INTERVAL * (int)i));

            m_sprFront->DrawImage(m_statusList.at(i).GetPercent(),
                                  STARTX,
                                  PADDING + STARTY + (INTERVAL * (int)i));
        }
    }
}

void NSHUD::HUD::OnDeviceLost()
{
    m_font->OnDeviceLost();
    m_sprBack->OnDeviceLost();
    m_sprFront->OnDeviceLost();
    m_sprMiddle->OnDeviceLost();
}

void NSHUD::HUD::OnDeviceReset()
{
    m_font->OnDeviceReset();
    m_sprBack->OnDeviceReset();
    m_sprFront->OnDeviceReset();
    m_sprMiddle->OnDeviceReset();
}

void NSHUD::StatusItem::SetName(const std::wstring& arg)
{
    m_name = arg;
}

std::wstring NSHUD::StatusItem::GetName() const
{
    return m_name;
}

void NSHUD::StatusItem::SetPercent(const int arg)
{
    m_percent = arg;
}

int NSHUD::StatusItem::GetPercent() const
{
    return m_percent;
}

void NSHUD::StatusItem::SetPercentSub(const int arg)
{
    m_percentSub = arg;
}

int NSHUD::StatusItem::GetPercentSub() const
{
    return m_percentSub;
}

void NSHUD::StatusItem::SetBarVisible(const bool arg)
{
    m_visible = arg;
}

bool NSHUD::StatusItem::GetBarVisible() const
{
    return m_visible;
}
