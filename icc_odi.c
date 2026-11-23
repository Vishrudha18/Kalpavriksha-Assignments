#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "players_data.h"

#define MAX_TEAMS 10
#define MAX_PLAYERS_PER_TEAM 50
#define NAME_LEN 50
#define ROLE_COUNT 3

typedef struct {
    int playerId;
    char name[NAME_LEN + 1];
    int teamId;
    int role; /* 1 = Batsman, 2 = Bowler, 3 = All-rounder */
    int totalRuns;
    float battingAverage;
    float strikeRate;
    int wickets;
    float economyRate;
    float performanceIndex;
} PlayerNode;

typedef struct PlayerListNode {
    PlayerNode data;
    struct PlayerListNode *teamNext;
    struct PlayerListNode *roleNext;
} PlayerListNode;

typedef struct {
    int teamId;
    char name[NAME_LEN + 1];
    int totalPlayers;
    float averageBattingStrikeRate;
    PlayerListNode *playersHead;
    PlayerListNode *playersTail;
    PlayerListNode *roleHead[ROLE_COUNT];
} Team;

typedef struct {
    PlayerListNode *node;
    int teamIndex;
} HeapItem;

static Team teamTable[MAX_TEAMS];
static const char *roleNames[ROLE_COUNT] = {
    "Batsman", "Bowler", "All-rounder"
};

static const int teamIds[MAX_TEAMS] = {7, 2, 12, 3, 1, 5, 4, 9, 10, 6};

static int getTeamIdByName(const char *name)
{
    int i;
    for (i = 0; i < teamCount; ++i) {
        if (strcmp(teamTable[i].name, name) == 0)
            return teamTable[i].teamId;
    }
    return -1;
}

static int teamIndexById(int teamId)
{
    int low = 0, high = teamCount - 1;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        if (teamTable[mid].teamId == teamId)
            return mid;
        if (teamTable[mid].teamId < teamId)
            low = mid + 1;
        else
            high = mid - 1;
    }
    return -1;
}

static float calculatePerformanceIndex(const PlayerNode *p)
{
    if (p->role == 1)
        return (p->battingAverage * p->strikeRate) / 100.0f;
    if (p->role == 2)
        return (p->wickets * 2.0f) + (100.0f - p->economyRate);
    return ((p->battingAverage * p->strikeRate) / 100.0f)
           + (p->wickets * 2.0f);
}

static int roleFromText(const char *role)
{
    int i;
    for (i = 0; i < ROLE_COUNT; ++i)
        if (strcmp(roleNames[i], role) == 0)
            return i + 1;
    return 0;
}

static int compareTeamsById(const void *a, const void *b)
{
    const Team *x = (const Team *)a;
    const Team *y = (const Team *)b;
    return (x->teamId > y->teamId) - (x->teamId < y->teamId);
}

static int playerIdExists(int playerId)
{
    int i;
    PlayerListNode *p;

    for (i = 0; i < teamCount; ++i) {
        for (p = teamTable[i].playersHead; p != NULL; p = p->teamNext) {
            if (p->data.playerId == playerId)
                return 1;
        }
    }
    return 0;
}

static void updateTeamAverage(Team *team)
{
    PlayerListNode *p;
    float total = 0.0f;
    int count = 0;

    for (p = team->playersHead; p != NULL; p = p->teamNext) {
        if (p->data.role == 1 || p->data.role == 3) {
            total += p->data.strikeRate;
            ++count;
        }
    }

    team->averageBattingStrikeRate = count ? total / count : 0.0f;
}

static int addPlayerToTeam(int teamId, const PlayerNode *player)
{
    int index = teamIndexById(teamId);
    Team *team;
    PlayerListNode *node, **position;

    if (index < 0)
        return 0;

    team = &teamTable[index];

    if (team->totalPlayers >= MAX_PLAYERS_PER_TEAM ||
        playerIdExists(player->playerId))
        return 0;

    node = (PlayerListNode *)malloc(sizeof(PlayerListNode));
    if (node == NULL)
        return 0;

    node->data = *player;
    node->teamNext = NULL;
    node->roleNext = NULL;

    if (team->playersTail == NULL)
        team->playersHead = team->playersTail = node;
    else {
        team->playersTail->teamNext = node;
        team->playersTail = node;
    }

    position = &team->roleHead[player->role - 1];
    while (*position != NULL &&
           (*position)->data.performanceIndex >= player->performanceIndex) {
        position = &(*position)->roleNext;
    }
    node->roleNext = *position;
    *position = node;

    ++team->totalPlayers;
    updateTeamAverage(team);
    return 1;
}

static void initializeTeams(void)
{
    int i;
    teamCount = (int)(sizeof(teams) / sizeof(teams[0]));
    for (i = 0; i < teamCount; ++i) {
        memset(&teamTable[i], 0, sizeof(Team));
        strncpy(teamTable[i].name, teams[i], NAME_LEN);
        teamTable[i].teamId = teamIds[i];
    }

    qsort(teamTable, teamCount, sizeof(Team), compareTeamsById);
}

static void initializePlayers(void)
{
    int i;
    for (i = 0; i < playerCount; ++i) {
        PlayerNode p;
        int role = roleFromText(players[i].role);
        int teamId = getTeamIdByName(players[i].team);

        if (role == 0 || teamId < 0)
            continue;

        memset(&p, 0, sizeof(p));
        p.playerId = players[i].id;
        strncpy(p.name, players[i].name, NAME_LEN);
        p.teamId = teamId;
        p.role = role;
        p.totalRuns = players[i].totalRuns;
        p.battingAverage = players[i].battingAverage;
        p.strikeRate = players[i].strikeRate;
        p.wickets = players[i].wickets;
        p.economyRate = players[i].economyRate;
        p.performanceIndex = calculatePerformanceIndex(&p);

        addPlayerToTeam(teamId, &p);
    }
}

static void freeAllPlayers(void)
{
    int i;
    for (i = 0; i < teamCount; ++i) {
        PlayerListNode *p = teamTable[i].playersHead;
        while (p != NULL) {
            PlayerListNode *next = p->teamNext;
            free(p);
            p = next;
        }
        teamTable[i].playersHead = NULL;
        teamTable[i].playersTail = NULL;
        for (int r = 0; r < ROLE_COUNT; ++r)
            teamTable[i].roleHead[r] = NULL;
    }
}

static void printPlayerHeader(void)
{
    printf("\n%-6s %-25s %-13s %-8s %-7s %-7s %-7s %-7s %-12s\n",
           "ID", "Name", "Role", "Runs", "Avg", "SR",
           "Wkts", "ER", "Perf.Index");
    printf("---------------------------------------------------------------------------------------------------------------\n");
}

static void printPlayer(const PlayerNode *p, int includeTeam)
{
    if (includeTeam) {
        printf("%-6d %-25s %-15s %-13s %-8d %-7.1f %-7.1f %-7d %-7.1f %-10.2f\n",
               p->playerId, p->name,
               teamTable[teamIndexById(p->teamId)].name,
               roleNames[p->role - 1], p->totalRuns,
               p->battingAverage, p->strikeRate, p->wickets,
               p->economyRate, p->performanceIndex);
    } else {
        printf("%-6d %-25s %-13s %-8d %-7.1f %-7.1f %-7d %-7.1f %-10.2f\n",
               p->playerId, p->name, roleNames[p->role - 1],
               p->totalRuns, p->battingAverage, p->strikeRate,
               p->wickets, p->economyRate, p->performanceIndex);
    }
}

static void displayPlayersOfTeam(void)
{
    int teamId, index;
    PlayerListNode *p;

    printf("Enter Team ID: ");
    if (scanf("%d", &teamId) != 1) {
        printf("Invalid input.\n");
        return;
    }

    index = teamIndexById(teamId);
    if (index < 0) {
        printf("Invalid Team ID!\n");
        return;
    }

    printf("\nPlayers of Team %s:\n", teamTable[index].name);
    printPlayerHeader();

    for (p = teamTable[index].playersHead; p != NULL; p = p->teamNext)
        printPlayer(&p->data, 0);

    printf("---------------------------------------------------------------------------------------------------------------\n");
    printf("Total Players: %d\n", teamTable[index].totalPlayers);
    printf("Average Batting Strike Rate: %.2f\n",
           teamTable[index].averageBattingStrikeRate);
}

typedef struct {
    int teamId;
    char name[NAME_LEN + 1];
    float average;
    int totalPlayers;
} TeamSummary;

static int compareTeamSummary(const void *a, const void *b)
{
    const TeamSummary *x = (const TeamSummary *)a;
    const TeamSummary *y = (const TeamSummary *)b;

    if (x->average < y->average) return 1;
    if (x->average > y->average) return -1;
    return x->teamId - y->teamId;
}

static void displayTeamsByAverageStrikeRate(void)
{
    TeamSummary summary[MAX_TEAMS];
    int i;

    for (i = 0; i < teamCount; ++i) {
        summary[i].teamId = teamTable[i].teamId;
        strcpy(summary[i].name, teamTable[i].name);
        summary[i].average = teamTable[i].averageBattingStrikeRate;
        summary[i].totalPlayers = teamTable[i].totalPlayers;
    }

    qsort(summary, teamCount, sizeof(TeamSummary), compareTeamSummary);

    printf("\nTeams Sorted by Average Batting Strike Rate\n");
    printf("---------------------------------------------------------------\n");
    printf("%-6s %-20s %-18s %-13s\n",
           "ID", "Team Name", "Avg Bat SR", "Total Players");
    printf("---------------------------------------------------------------\n");

    for (i = 0; i < teamCount; ++i) {
        printf("%-6d %-20s %-18.2f %-13d\n",
               summary[i].teamId, summary[i].name,
               summary[i].average, summary[i].totalPlayers);
    }
    printf("---------------------------------------------------------------\n");
}

static void addPlayerMenu(void)
{
    PlayerNode p;
    int teamId, role;

    memset(&p, 0, sizeof(p));

    printf("Enter Team ID to add player: ");
    if (scanf("%d", &teamId) != 1) {
        printf("Invalid input.\n");
        return;
    }
    if (teamIndexById(teamId) < 0) {
        printf("Invalid Team ID!\n");
        return;
    }

    printf("\nEnter Player Details:\n");
    printf("Player ID: ");
    if (scanf("%d", &p.playerId) != 1 || p.playerId < 1 || p.playerId > 1000) {
        printf("Invalid Player ID!\n");
        return;
    }
    if (playerIdExists(p.playerId)) {
        printf("Player ID already exists!\n");
        return;
    }

    printf("Name: ");
    if (scanf(" %50[^\n]", p.name) != 1) {
        printf("Invalid name.\n");
        return;
    }

    printf("Role (1-Batsman, 2-Bowler, 3-All-rounder): ");
    if (scanf("%d", &role) != 1 || role < 1 || role > 3) {
        printf("Invalid role!\n");
        return;
    }
    p.role = role;

    printf("Total Runs: ");
    if (scanf("%d", &p.totalRuns) != 1 || p.totalRuns < 0) {
        printf("Invalid total runs.\n");
        return;
    }
    printf("Batting Average: ");
    if (scanf("%f", &p.battingAverage) != 1 || p.battingAverage < 0) {
        printf("Invalid batting average.\n");
        return;
    }
    printf("Strike Rate: ");
    if (scanf("%f", &p.strikeRate) != 1 || p.strikeRate < 0) {
        printf("Invalid strike rate.\n");
        return;
    }
    printf("Wickets: ");
    if (scanf("%d", &p.wickets) != 1 || p.wickets < 0) {
        printf("Invalid wickets.\n");
        return;
    }
    printf("Economy Rate: ");
    if (scanf("%f", &p.economyRate) != 1 || p.economyRate < 0) {
        printf("Invalid economy rate.\n");
        return;
    }

    p.teamId = teamId;
    p.performanceIndex = calculatePerformanceIndex(&p);

    if (addPlayerToTeam(teamId, &p))
        printf("Player added successfully to Team %s!\n",
               teamTable[teamIndexById(teamId)].name);
    else
        printf("Unable to add player. Team may be full or memory allocation failed.\n");
}

static void displayTopKPlayers(void)
{
    int teamId, role, k, index, count = 0;
    PlayerListNode *p;

    printf("Enter Team ID: ");
    if (scanf("%d", &teamId) != 1) {
        printf("Invalid input.\n");
        return;
    }
    index = teamIndexById(teamId);
    if (index < 0) {
        printf("Invalid Team ID!\n");
        return;
    }

    printf("Enter Role (1-Batsman, 2-Bowler, 3-All-rounder): ");
    if (scanf("%d", &role) != 1 || role < 1 || role > 3) {
        printf("Invalid role!\n");
        return;
    }

    printf("Enter number of players: ");
    if (scanf("%d", &k) != 1 || k <= 0) {
        printf("Invalid number of players.\n");
        return;
    }

    printf("\nTop %d %s of Team %s:\n", k, roleNames[role - 1],
           teamTable[index].name);
    printPlayerHeader();

    for (p = teamTable[index].roleHead[role - 1];
         p != NULL && count < k; p = p->roleNext, ++count)
        printPlayer(&p->data, 0);

    if (count == 0)
        printf("No players found for this role.\n");
}

static void heapSwap(HeapItem *a, HeapItem *b)
{
    HeapItem temp = *a;
    *a = *b;
    *b = temp;
}

static int higherPerformance(const HeapItem *a, const HeapItem *b)
{
    if (a->node->data.performanceIndex != b->node->data.performanceIndex)
        return a->node->data.performanceIndex > b->node->data.performanceIndex;
    return a->node->data.playerId < b->node->data.playerId;
}

static void heapPush(HeapItem heap[], int *size, HeapItem item)
{
    int i = (*size)++;
    heap[i] = item;

    while (i > 0) {
        int parent = (i - 1) / 2;
        if (higherPerformance(&heap[parent], &heap[i]))
            break;
        heapSwap(&heap[parent], &heap[i]);
        i = parent;
    }
}

static HeapItem heapPop(HeapItem heap[], int *size)
{
    HeapItem result = heap[0];
    int i = 0;

    --(*size);
    if (*size == 0)
        return result;

    heap[0] = heap[*size];

    while (1) {
        int left = 2 * i + 1;
        int right = left + 1;
        int best = i;

        if (left < *size && higherPerformance(&heap[left], &heap[best]))
            best = left;
        if (right < *size && higherPerformance(&heap[right], &heap[best]))
            best = right;
        if (best == i)
            break;

        heapSwap(&heap[i], &heap[best]);
        i = best;
    }

    return result;
}

static void displayAllTeamsByRole(void)
{
    int role, i, heapSize = 0;
    HeapItem heap[MAX_TEAMS];

    printf("Enter Role (1-Batsman, 2-Bowler, 3-All-rounder): ");
    if (scanf("%d", &role) != 1 || role < 1 || role > 3) {
        printf("Invalid role!\n");
        return;
    }

    for (i = 0; i < teamCount; ++i) {
        if (teamTable[i].roleHead[role - 1] != NULL) {
            HeapItem item;
            item.node = teamTable[i].roleHead[role - 1];
            item.teamIndex = i;
            heapPush(heap, &heapSize, item);
        }
    }

    printf("\nAll %s players across teams (descending performance index):\n",
           roleNames[role - 1]);
    printf("%-6s %-25s %-15s %-13s %-8s %-7s %-7s %-7s %-7s %-10s\n",
           "ID", "Name", "Team", "Role", "Runs", "Avg",
           "SR", "Wkts", "ER", "Perf.Index");
    printf("---------------------------------------------------------------------------------------------------------------------------------\n");

    while (heapSize > 0) {
        HeapItem item = heapPop(heap, &heapSize);
        printPlayer(&item.node->data, 1);

        if (item.node->roleNext != NULL) {
            HeapItem nextItem;
            nextItem.node = item.node->roleNext;
            nextItem.teamIndex = item.teamIndex;
            heapPush(heap, &heapSize, nextItem);
        }
    }
    printf("---------------------------------------------------------------------------------------------------------------------------------\n");
}

static void displayMenu(void)
{
    printf("\n===============================================================\n");
    printf("             ICC ODI Player Performance Analyzer\n");
    printf("===============================================================\n");
    printf("1. Add Player to Team\n");
    printf("2. Display Players of a Specific Team\n");
    printf("3. Display Teams by Average Batting Strike Rate\n");
    printf("4. Display Top K Players of a Specific Team by Role\n");
    printf("5. Display All Players of Specific Role Across All Teams\n");
    printf("6. Exit\n");
    printf("===============================================================\n");
    printf("Enter your choice: ");
}

int main(void)
{
    int choice;

    initializeTeams();
    initializePlayers();

    do {
        displayMenu();
        if (scanf("%d", &choice) != 1) {
            printf("\nInvalid input. Exiting.\n");
            break;
        }

        switch (choice) {
            case 1:
                addPlayerMenu();
                break;
            case 2:
                displayPlayersOfTeam();
                break;
            case 3:
                displayTeamsByAverageStrikeRate();
                break;
            case 4:
                displayTopKPlayers();
                break;
            case 5:
                displayAllTeamsByRole();
                break;
            case 6:
                printf("Exiting ICC ODI Player Performance Analyzer.\n");
                break;
            default:
                printf("Invalid choice! Please enter a number from 1 to 6.\n");
        }
    } while (choice != 6);

    freeAllPlayers();
    return 0;
}
