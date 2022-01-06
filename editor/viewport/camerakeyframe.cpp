#include "camerakeyframe.h"
#include "../zenvis/zenvis.h"

CameraKeyframeWidget::CameraKeyframeWidget()
    : m_list(nullptr)
    , m_enable(nullptr)
    , m_key(nullptr)
    , m_remove(nullptr)
{
}

bool CameraKeyframeWidget::queryFrame(int frame, PerspectiveInfo& ret)
{
    return false;
}

void CameraKeyframeWidget::insertKeyFrames()
{
    int frameid = Zenvis::GetInstance().getCurrentFrameId();
    m_keyFrames[frameid] = Zenvis::GetInstance().m_perspective;
    updateList();
}

void CameraKeyframeWidget::removeKeyFrame()
{
    int sel = m_list->currentRow();
    if (sel < 0)
        return;

    //sort(m_keyFrames.begin(), m_keyFrames.end(), [=]() {});
    //...
    updateList();
}

void CameraKeyframeWidget::updateList()
{
    m_list->clear();
    //...
}