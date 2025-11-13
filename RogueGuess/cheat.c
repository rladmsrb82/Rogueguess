// cheat.c
#include <stdio.h>
#include "cheat.h"

void open_cheat_menu(Player* p) {
    int running = 1;

    while (running) {
        printf("===== CHEAT MENU =====\n");
        printf("1. 돈 +10000\n");
        printf("2. 특성 모두 만렙\n");
        printf("0. 돌아가기\n");
        printf("======================\n");
        printf("선택: ");

        int choice;
        if (scanf_s("%d", &choice) != 1) {
            printf("잘못된 입력입니다.\n");
            int c;
            while ((c = getchar()) != '\n' && c != EOF) {}
            continue;
        }
        int c;
        while ((c = getchar()) != '\n' && c != EOF) {}

        switch (choice) {
        case 0:
            printf("치트 메뉴를 종료합니다.\n");
            running = 0;
            break;

        case 1:
            p->gold += 10000;
            printf("치트 발동! Gold +10000 (현재 Gold: %d)\n", p->gold);
            break;

        case 2:
            // 특성 전부 최대로
            p->trait_hp_level = 4; // 체력 +4
            p->trait_range_level = 3; // ±3 판정
            p->trait_gold_bonus_level = 3; // +30 골드 보너스
            p->trait_exact_level = 1; // 정확한 감각 ON
            p->trait_insight_level = 1; // 날카로운 직감 패시브 ON
            p->trait_second_chance_level = 1; // 두 번째 기회 패시브 ON

            printf("치트 발동! 특성 모두 만렙 상태가 되었습니다.\n");
            printf("(체력, 판정 범위, 골드 보너스, 정확한 감각, 직감, 두 번째 기회)\n");
            break;

        default:
            printf("0~2 중에서 선택하세요.\n");
            break;
        }

        printf("\n");
    }
}
