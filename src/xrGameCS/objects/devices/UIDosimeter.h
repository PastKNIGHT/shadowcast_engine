#pragma once

#include "../../ui/ArtefactDetectorUI.h"
#include "Dosimeter.h"

class CUIDosimeter : public CUIArtefactDetectorBase, public CUIWindow
{
public:
    void update() override;
    void Draw() override;

    void construct(CDosimeter* p);

private:
    CUIStatic* m_wrk_area;
    CUIStatic* m_seg1;
    CUIStatic* m_seg2;
    CUIStatic* m_seg3;
    CUIStatic* m_seg4;
    CDosimeter* m_parent;
    Fmatrix m_map_attach_offset;

    void GetUILocatorMatrix(Fmatrix& _m);

    // ������� ������ �������: �������� ����� � ������ ������ ����
    CUIStatic* m_workIndicator;
    const u32 WORK_PERIOD = 1000; // ������ ������� ����������
    u32 m_workTick; // ����� ������������ ����������

    // �������� ���� ��� ���������: ������� ������ �������� � �������� 8 ������
    float m_noise; // �������� ����
    const u32 NOISE_PERIOD = 3000; // ������ ����������� ����
    u32 m_noiseTick; // ����� ���������� ����������� ����
};