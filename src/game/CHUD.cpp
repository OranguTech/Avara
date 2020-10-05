#include "CHUD.h"
#include "CAbstractPlayer.h"
#include "CAvaraGame.h"
#include "CPlayerManager.h"
#include "AvaraDefines.h"
#include "CScoreKeeper.h"


CHUD::CHUD(CAvaraGame *game) {
    itsGame = game;
}

const int CHAT_CHARS = 40;
const NVGcolor BACKGROUND_COLOR = nvgRGBA(30, 30, 30, 180);

const std::vector<long> team_colors =
    {kGreenTeamColor, kYellowTeamColor, kRedTeamColor, kPinkTeamColor, kPurpleTeamColor, kBlueTeamColor, kOrangeTeamColor, kLimeTeamColor};


bool sortByScore(std::pair<PlayerScoreRecord, int> i, std::pair<PlayerScoreRecord, int> j) {
    if(i.first.points == j.first.points) {
        return i.second < j.second;
    }
    else {
        return i.first.points > j.first.points;
    }
}

void CHUD::DrawScore(int playingCount, int chudHeight, CViewParameters *view, NVGcontext *ctx) {
    CAbstractPlayer *player = itsGame->GetLocalPlayer();
    CPlayerManager *playerManager = itsGame->FindPlayerManager(player);

    if(playingCount > 0 && (playerManager->GetShowScoreboard() || itsGame->gameStatus != kPlayingStatus)) {
        AvaraScoreRecord theScores = itsGame->scoreKeeper->localScores;
        CNetManager *net = itsGame->itsApp->GetNet();
        float colorBoxWidth = 30.0;
        int bufferWidth = view->viewPixelDimensions.h;
        int bufferHeight = view->viewPixelDimensions.v;
        float boardWidth = 620;
        float boardHeight = 60 + (colorBoxWidth + 10) * playingCount;
        float x = 20;
        float y = bufferHeight-chudHeight-boardHeight - 20;
        float fontsz_m = 28.0, fontsz_s = 18.0;
        long longTeamColor;
        int colorR, colorG, colorB;
        NVGcolor aliveColor = nvgRGBA(255, 255, 255, 255);
        NVGcolor deadColor = nvgRGBA(165, 165, 165, 255);
        NVGcolor highColor = nvgRGBA(255, 0, 0, 255);
        float colWidth = 80;
        float rankWidth = 40;
        
        //use netscores when not in the game
        if(itsGame->gameStatus != kPlayingStatus && itsGame->gameStatus != kPauseStatus) {
            theScores = itsGame->scoreKeeper->netScores;
        }
        
        //sort by highscore
        int16_t highKills = 0;
        std::vector<std::pair<PlayerScoreRecord, int> > sortedPlayers;
        for (int i = 0; i < kMaxAvaraPlayers; ++i) {
            sortedPlayers.push_back(std::make_pair(theScores.player[i], i));
            if(net->playerTable[i]->GetPlayer())
                highKills = std::max(highKills, theScores.player[i].kills);
        }
        std::sort(sortedPlayers.begin(), sortedPlayers.end(), sortByScore);

        //draw score box
        nvgBeginPath(ctx);
        nvgFillColor(ctx, nvgRGBA(20, 20, 20, 160));
        nvgRoundedRect(ctx, x, y, boardWidth, boardHeight, 4.0);
        nvgFill(ctx);
        
        //header box
        nvgBeginPath(ctx);
        nvgFillColor(ctx, nvgRGBA(0, 0, 0, 160));
        nvgRoundedRect(ctx, x, y, boardWidth, 40, 4.0);
        nvgFill(ctx);
        
        //drop shadow
        NVGpaint shadowPaint = nvgBoxGradient(ctx, x,y+2, boardWidth,boardHeight, 8, 10, nvgRGBA(0,0,0,128), nvgRGBA(0,0,0,0));
        nvgBeginPath(ctx);
        nvgRect(ctx, x-10,y-10, boardWidth+20,boardHeight+30);
        nvgRoundedRect(ctx, x,y, boardWidth,boardHeight, 4.0);
        nvgPathWinding(ctx, NVG_HOLE);
        nvgFillPaint(ctx, shadowPaint);
        nvgFill(ctx);
        
        x+=10;
        
        //score column text settings
        nvgFillColor(ctx, aliveColor);
        nvgTextAlign(ctx, NVG_ALIGN_RIGHT | NVG_ALIGN_TOP);
        nvgFontSize(ctx, fontsz_s);
        
        //column titles
        y+= 10;
        nvgText(ctx, x + colorBoxWidth + colWidth + 10, y, "Player", NULL);
        nvgText(ctx, x + colorBoxWidth + colWidth*4, y, "Score", NULL);
        nvgText(ctx, x + colorBoxWidth + colWidth*5, y, "Kills", NULL);
        nvgText(ctx, x + colorBoxWidth + colWidth*6, y, "Lives", NULL);
        nvgText(ctx, x + colorBoxWidth + colWidth*7, y, "RT(ms)", NULL);
        y+= 45;

        int playerRank = 0;
        nvgFontSize(ctx, fontsz_m);
        int16_t previousScore = -32768;
        for (int i = 0; i < kMaxAvaraPlayers; i++) {
            int playerTableIndex = sortedPlayers[i].second;
            CPlayerManager *thisPlayer = net->playerTable[playerTableIndex];
            const std::string playerName((char *)thisPlayer->PlayerName() + 1, thisPlayer->PlayerName()[0]);
            std::string ping = "--";
            longTeamColor = team_colors[net->teamColors[playerTableIndex]];
            colorR = (longTeamColor >> 16) & 0xff;
            colorG = (longTeamColor >> 8) & 0xff;
            colorB = longTeamColor & 0xff;
            NVGcolor textColor = aliveColor;
            
            if(playerName.size() > 0 && thisPlayer->GetPlayer() != NULL) {
                if(theScores.player[playerTableIndex].points != previousScore)
                    playerRank++;
                
                //int playerLives = thisPlayer->GetPlayer()->lives;
                int playerLives = theScores.player[playerTableIndex].lives;
                if(thisPlayer->IsLocalPlayer()) {
                    //self highlight
                    nvgBeginPath(ctx);
                    nvgRoundedRect(ctx, x - 5, y-3, boardWidth - 10, colorBoxWidth + 6, 4.0);
                    nvgFillColor(ctx, nvgRGBA(128, 255, 0, 210));
                    nvgFill(ctx);
                }
                else {
                    ping = std::to_string(net->itsCommManager->GetMaxRoundTrip(1 << playerTableIndex));
                }
                
                //player color box
                nvgBeginPath(ctx);
                nvgRoundedRect(ctx, x + rankWidth + 10, y, colorBoxWidth, colorBoxWidth, 3.0);
                nvgFillColor(ctx, nvgRGBA(colorR, colorG, colorB, 255));
                nvgFill(ctx);
                
                //show winnner
                if(thisPlayer->WinFrame() > 0) {
                    std::string checkMark("\u221A");
                    nvgFillColor(ctx, aliveColor);
                    nvgText(ctx, x + rankWidth + 33, y, checkMark.c_str(), NULL);
                }
                
                //score text settings
                textColor = aliveColor;
                if(playerLives == 0 && (itsGame->gameStatus == kPlayingStatus || itsGame->gameStatus == kPauseStatus))
                    textColor = deadColor;
                
                nvgFillColor(ctx, textColor);
                nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
                nvgFontSize(ctx, fontsz_m);

                //rank
                std::string rank = std::to_string(playerRank);
                switch (playerRank) {
                    case 1: rank += "ST"; break;
                    case 2: rank += "ND"; break;
                    case 3: rank += "RD"; break;
                    default: rank += "TH"; break;
                }
                nvgText(ctx, x, y, rank.c_str(), NULL);

                //player name and scores
                nvgText(ctx, x + colorBoxWidth + rankWidth + 15, y, playerName.c_str(), NULL);
                
                nvgTextAlign(ctx, NVG_ALIGN_RIGHT | NVG_ALIGN_TOP);
                nvgText(ctx, x + colorBoxWidth + colWidth*4, y, std::to_string(theScores.player[playerTableIndex].points).c_str(), NULL);

                //kills
                if(highKills > 0 && theScores.player[playerTableIndex].kills == highKills)
                    nvgFillColor(ctx, highColor);
                nvgText(ctx, x + colorBoxWidth + colWidth*5, y, std::to_string(theScores.player[playerTableIndex].kills).c_str(), NULL);
                nvgFillColor(ctx, textColor);

                nvgText(ctx, x + colorBoxWidth + colWidth*6, y, std::to_string(playerLives).c_str(), NULL);
                nvgText(ctx, x + colorBoxWidth + colWidth*7, y, ping.c_str(), NULL);

                y += colorBoxWidth + 10;
                
                previousScore = theScores.player[playerTableIndex].points;
             }
        }
    }
}

void CHUD::DrawLevelName(CViewParameters *view, NVGcontext *ctx) {
    std::string level((char *)itsGame->loadedLevel + 1, itsGame->loadedLevel[0]);
    if(itsGame->gameStatus != kPlayingStatus && level.length() > 0) {
        int bufferWidth = view->viewPixelDimensions.h;
        int bufferHeight = view->viewPixelDimensions.v;
        float x = 0.0;
        float y = bufferHeight - 130.0;
        float bounds[4];
        
        nvgBeginPath(ctx);
        nvgFontFace(ctx, "mono");
        nvgFontSize(ctx, 24.0);
        nvgTextBounds(ctx, x,y, level.c_str(), NULL, bounds);
        x = (bufferWidth / 2) - (bounds[2]-bounds[0]) / 2;
        nvgFillColor(ctx, BACKGROUND_COLOR);
        nvgRect(ctx, x-5,y, (int)(bounds[2]-bounds[0])+10, (int)(bounds[3]-bounds[1])+4);
        nvgFill(ctx);
        
        nvgFillColor(ctx, nvgRGBA(255,255,255,220));
        nvgText(ctx, x,y+(bounds[3]-bounds[1])-3, level.c_str(), NULL);
    }
}

void CHUD::DrawPaused(CViewParameters *view, NVGcontext *ctx) {
    if(itsGame->gameStatus == kPauseStatus) {
        int bufferWidth = view->viewPixelDimensions.h;
        int bufferHeight = view->viewPixelDimensions.v;
        int centerX = bufferWidth / 2;
        int centerY = bufferHeight / 2;
        float barWidth = 25.0;
        float barHeight = 100.0;

        //draw circle
        nvgBeginPath(ctx);
        nvgCircle(ctx, centerX, centerY, 100.0);
        nvgFillColor(ctx, nvgRGBA(0,0,0,180));
        nvgFill(ctx);

        //vertical pause bars
        nvgBeginPath(ctx);
        nvgRoundedRect(ctx, centerX + 20, centerY - (barHeight/2), barWidth, barHeight, 10);
        nvgRoundedRect(ctx, centerX - 20 - barWidth, centerY - (barHeight/2), barWidth, barHeight, 10);
        nvgFillColor(ctx, nvgRGBA(255,255,255,180));
        nvgFill(ctx);
    }
}

void CHUD::Render(CViewParameters *view, NVGcontext *ctx) {
    CAbstractPlayer *player = itsGame->GetLocalPlayer();
    CAbstractPlayer *spectatePlayer = itsGame->GetSpectatePlayer();
    CNetManager *net = itsGame->itsApp->GetNet();

    DrawLevelName(view, ctx);
    DrawPaused(view, ctx);
    
    int playerCount = 0;
    int playingCount = 0;
    for (int i = 0; i < kMaxAvaraPlayers; i++) {
        CPlayerManager *thisPlayer = net->playerTable[i];
        std::string playerName((char *)thisPlayer->PlayerName() + 1, thisPlayer->PlayerName()[0]);
        if (playerName.length() > 0) {
            playerCount++;
    
            if(thisPlayer->GetPlayer() != NULL) {
                playingCount++;
            }
        }
    }
    int playerSlots = std::max(6, playerCount);

    int bufferWidth = view->viewPixelDimensions.h, bufferHeight = view->viewPixelDimensions.v;
    int chudHeight = 13 * playerSlots;
    
    DrawScore(playingCount, chudHeight, view, ctx);
    
    nvgBeginFrame(ctx, bufferWidth, bufferHeight, view->viewPixelRatio);

    nvgBeginPath(ctx);
    nvgRect(ctx, 0, bufferHeight - chudHeight, bufferWidth, chudHeight);
    nvgFillColor(ctx, BACKGROUND_COLOR);
    nvgFill(ctx);

    float fontsz_m = 15.0, fontsz_s = 10.0;
    nvgFontFace(ctx, "mono");

    float mY = (bufferHeight - chudHeight + 8);
    for (auto i : itsGame->itsApp->MessageLines()) {
        nvgBeginPath(ctx);
        nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
        nvgFontSize(ctx, fontsz_m);
        nvgFillColor(ctx, nvgRGBA(255, 255, 255, 255));
        nvgText(ctx, 20, mY, i.c_str(), NULL);
        mY += 11;
    }

    float pY;
    long longTeamColor;
    int colorR, colorG, colorB;
    for (int i = 0; i < kMaxAvaraPlayers; i++) {
        CPlayerManager *thisPlayer = net->playerTable[i];
        std::string playerName((char *)thisPlayer->PlayerName() + 1, thisPlayer->PlayerName()[0]);
        if (playerName.length() < 1) continue;
        pY = (bufferHeight - chudHeight + 8) + (11 * i);
        longTeamColor = team_colors[net->teamColors[i]];
        colorR = (longTeamColor >> 16) & 0xff;
        colorG = (longTeamColor >> 8) & 0xff;
        colorB = longTeamColor & 0xff;
        std::string playerChat = thisPlayer->GetChatString(CHAT_CHARS);

        //player color box
        NVGcolor textColor = nvgRGBA(255, 255, 255, 255);
        float colorBoxWidth = 10.0;
        nvgBeginPath(ctx);
        
        //highlight player if spectating
        if(spectatePlayer != NULL && thisPlayer->GetPlayer() == spectatePlayer) {
            textColor = nvgRGBA(0, 0, 0, 255);
            colorBoxWidth = 150.0;
        }

        nvgRect(ctx, bufferWidth - 160, pY, colorBoxWidth, 10.0);
        nvgFillColor(ctx, nvgRGBA(colorR, colorG, colorB, 255));
        nvgFill(ctx);
        
        //player name
        nvgFillColor(ctx, textColor);
        nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
        nvgFontSize(ctx, fontsz_m);
        nvgText(ctx, bufferWidth - 148, pY - 3, playerName.c_str(), NULL);
        
        short status = thisPlayer->GetStatusChar();
        if (status >= 0) {
            std::string playerLives = std::to_string(status);
            if (status == 10) playerLives = "%";
            if (status == 12) playerLives = "C";
            nvgTextAlign(ctx, NVG_ALIGN_RIGHT | NVG_ALIGN_TOP);
            nvgFontSize(ctx, fontsz_s);
            nvgFillColor(ctx, nvgRGBA(255, 255, 255, 255));
            nvgText(ctx, bufferWidth - 162, pY - 3, playerLives.c_str(), NULL);
            if (thisPlayer->GetMessageIndicator() > 0) {
                nvgText(ctx, bufferWidth - 162, pY + 3, "<", NULL);
            }
        }

        nvgTextAlign(ctx, NVG_ALIGN_RIGHT | NVG_ALIGN_TOP);
        nvgFontSize(ctx, fontsz_m);
        nvgFillColor(ctx, nvgRGBA(255, 255, 255, 255));
        nvgText(ctx, bufferWidth - 168, pY - 3, playerChat.c_str(), NULL);
        
        //spectating onscreen name
        if(spectatePlayer != NULL && thisPlayer->GetPlayer() == spectatePlayer) {
            int x = 20;
            int y = 20;
            float fontsz_m = 24.0;
            float bounds[4];
            std::string specMessage("Spectating " + playerName);

            nvgBeginPath(ctx);
            nvgFontFace(ctx, "mono");
            nvgTextAlign(ctx, NVG_ALIGN_MIDDLE | NVG_ALIGN_BOTTOM);
            nvgFontSize(ctx, fontsz_m);
            nvgTextBounds(ctx, x,y, specMessage.c_str(), NULL, bounds);

            //draw box for text
            nvgBeginPath(ctx);
            nvgRoundedRect(ctx, x, y, (bounds[2]-bounds[0])+10, 28.0, 3.0);
            nvgFillColor(ctx, BACKGROUND_COLOR);
            nvgFill(ctx);
            
            //draw text
            nvgFillColor(ctx, nvgRGBA(255, 255, 255, 255));
            nvgText(ctx, x + 5, y + 14, specMessage.c_str(), NULL);
        }
    }

    if (!player)
        return;
    
    if(spectatePlayer != NULL)
        player = spectatePlayer;
    
    int i, j;
    float g1X = (bufferWidth / 2.0) - 60.0;
    float gY = bufferHeight - 60.0;
    float g2X1 = (bufferWidth / 2.0) + 15.0;
    float g2Y1 = bufferHeight - 60.0;
    float g2X2 = (bufferWidth / 2.0) + 15.0 + 22.5;
    float g2Y2 = bufferHeight - 60.0 + 7.5;
    float g2X3 = (bufferWidth / 2.0) + 15.0 + 15.0;
    float g2Y3 = bufferHeight - 60.0 + 30.0;
    float g2X4 = (bufferWidth / 2.0) + 15.0 - 7.5;
    float g2Y4 = bufferHeight - 60.0 + 22.5;
    char scoreText[20];
    char timeText[9];
    float full = 30.0;
    float half = 15.0;
    float w_pow = 15.0;
    int secs, mins;
    long timeTemp = itsGame->timeInSeconds;
    float energy = sqrt((float)player->energy / (float)player->maxEnergy) * full;
    float pow_l = ((float)player->gunEnergy[1] / (float)player->fullGunEnergy) * half;
    float pow_r = ((float)player->gunEnergy[0] / (float)player->fullGunEnergy) * half;
    float shields = sqrt((float)player->shields / (float)player->maxShields) * full;
    float g2s = 3.75;
    float g1[4][4][2] = {{{g1X, gY}, {g1X, gY + energy}, {g1X + energy, gY + energy}, {g1X, gY}},
        {{g1X + half, gY + half},
            {g1X + half - pow_l, gY + half - pow_l},
            {g1X + half - pow_l + w_pow, gY + half - pow_l},
            {g1X + half + w_pow, gY + half}},
        {{g1X + half, gY + half},
            {g1X + half + pow_r, gY + half + pow_r},
            {g1X + half + pow_r + w_pow, gY + half + pow_r},
            {g1X + half + w_pow, gY + half}},
        {{g1X + w_pow + full - shields, gY + full - shields},
            {g1X + full + w_pow, gY + full},
            {g1X + full + w_pow, gY + full},
            {g1X + w_pow + full, gY + full - shields}}};
    float g2[4][4][4][2] = {{// boosters
                                {{g2X1 + (g2s * 0), g2Y1 + (g2s * 0)},
                                    {g2X1 + (g2s * 1), g2Y1 + (g2s * 1)},
                                    {g2X1 + (g2s * 7), g2Y1 + (g2s * 1)},
                                    {g2X1 + (g2s * 8), g2Y1 + (g2s * 0)}},
                                {{g2X1 + (g2s * 1), g2Y1 + (g2s * 1)},
                                    {g2X1 + (g2s * 2), g2Y1 + (g2s * 2)},
                                    {g2X1 + (g2s * 6), g2Y1 + (g2s * 2)},
                                    {g2X1 + (g2s * 7), g2Y1 + (g2s * 1)}},
                                {{g2X1 + (g2s * 2), g2Y1 + (g2s * 2)},
                                    {g2X1 + (g2s * 3), g2Y1 + (g2s * 3)},
                                    {g2X1 + (g2s * 5), g2Y1 + (g2s * 3)},
                                    {g2X1 + (g2s * 6), g2Y1 + (g2s * 2)}},
                                {{g2X1 + (g2s * 3), g2Y1 + (g2s * 3)},
                                    {g2X1 + (g2s * 4), g2Y1 + (g2s * 4)},
                                    {g2X1 + (g2s * 4), g2Y1 + (g2s * 4)},
                                    {g2X1 + (g2s * 5), g2Y1 + (g2s * 3)}}},
        {// grenades
            {{g2X2 - (g2s * 0), g2Y2 + (g2s * 0)},
                {g2X2 - (g2s * 1), g2Y2 + (g2s * 1)},
                {g2X2 - (g2s * 1), g2Y2 + (g2s * 7)},
                {g2X2 - (g2s * 0), g2Y2 + (g2s * 8)}},
            {{g2X2 - (g2s * 1), g2Y2 + (g2s * 1)},
                {g2X2 - (g2s * 2), g2Y2 + (g2s * 2)},
                {g2X2 - (g2s * 2), g2Y2 + (g2s * 6)},
                {g2X2 - (g2s * 1), g2Y2 + (g2s * 7)}},
            {{g2X2 - (g2s * 2), g2Y2 + (g2s * 2)},
                {g2X2 - (g2s * 3), g2Y2 + (g2s * 3)},
                {g2X2 - (g2s * 3), g2Y2 + (g2s * 5)},
                {g2X2 - (g2s * 2), g2Y2 + (g2s * 6)}},
            {{g2X2 - (g2s * 3), g2Y2 + (g2s * 3)},
                {g2X2 - (g2s * 4), g2Y2 + (g2s * 4)},
                {g2X2 - (g2s * 4), g2Y2 + (g2s * 4)},
                {g2X2 - (g2s * 3), g2Y2 + (g2s * 5)}}},
        {// lives
            {{g2X3 - (g2s * 0), g2Y3 - (g2s * 0)},
                {g2X3 - (g2s * 1), g2Y3 - (g2s * 1)},
                {g2X3 - (g2s * 7), g2Y3 - (g2s * 1)},
                {g2X3 - (g2s * 8), g2Y3 - (g2s * 0)}},
            {{g2X3 - (g2s * 1), g2Y3 - (g2s * 1)},
                {g2X3 - (g2s * 2), g2Y3 - (g2s * 2)},
                {g2X3 - (g2s * 6), g2Y3 - (g2s * 2)},
                {g2X3 - (g2s * 7), g2Y3 - (g2s * 1)}},
            {{g2X3 - (g2s * 2), g2Y3 - (g2s * 2)},
                {g2X3 - (g2s * 3), g2Y3 - (g2s * 3)},
                {g2X3 - (g2s * 5), g2Y3 - (g2s * 3)},
                {g2X3 - (g2s * 6), g2Y3 - (g2s * 2)}},
            {{g2X3 - (g2s * 3), g2Y3 - (g2s * 3)},
                {g2X3 - (g2s * 4), g2Y3 - (g2s * 4)},
                {g2X3 - (g2s * 4), g2Y3 - (g2s * 4)},
                {g2X3 - (g2s * 5), g2Y3 - (g2s * 3)}}},
        {// missiles
            {{g2X4 + (g2s * 0), g2Y4 - (g2s * 0)},
                {g2X4 + (g2s * 1), g2Y4 - (g2s * 1)},
                {g2X4 + (g2s * 1), g2Y4 - (g2s * 7)},
                {g2X4 + (g2s * 0), g2Y4 - (g2s * 8)}},
            {{g2X4 + (g2s * 1), g2Y4 - (g2s * 1)},
                {g2X4 + (g2s * 2), g2Y4 - (g2s * 2)},
                {g2X4 + (g2s * 2), g2Y4 - (g2s * 6)},
                {g2X4 + (g2s * 1), g2Y4 - (g2s * 7)}},
            {{g2X4 + (g2s * 2), g2Y4 - (g2s * 2)},
                {g2X4 + (g2s * 3), g2Y4 - (g2s * 3)},
                {g2X4 + (g2s * 3), g2Y4 - (g2s * 5)},
                {g2X4 + (g2s * 2), g2Y4 - (g2s * 6)}},
            {{g2X4 + (g2s * 3), g2Y4 - (g2s * 3)},
                {g2X4 + (g2s * 4), g2Y4 - (g2s * 4)},
                {g2X4 + (g2s * 4), g2Y4 - (g2s * 4)},
                {g2X4 + (g2s * 3), g2Y4 - (g2s * 5)}}}};
    NVGcolor g1c[] = {
        nvgRGBA(0, 143, 0, 255), nvgRGBA(163, 54, 0, 255), nvgRGBA(255, 54, 0, 255), nvgRGBA(0, 61, 165, 255)};
    NVGcolor g2c[] = {
        nvgRGBA(30, 30, 102, 255), nvgRGBA(30, 30, 153, 255), nvgRGBA(30, 30, 204, 255), nvgRGBA(30, 30, 255, 255)};
    for (i = 0; i < 4; i++) { // referred to as GrafPanel in original Avara
        nvgBeginPath(ctx);
        nvgMoveTo(ctx, g1[i][0][0], g1[i][0][1]);
        nvgLineTo(ctx, g1[i][1][0], g1[i][1][1]);
        nvgLineTo(ctx, g1[i][2][0], g1[i][2][1]);
        nvgLineTo(ctx, g1[i][3][0], g1[i][3][1]);
        nvgClosePath(ctx);
        nvgFillColor(ctx, g1c[i]);
        nvgFill(ctx);
    }
    for (i = 0; i < 4; i++) { // "pinwheel" indicator
        for (j = 0; j < 4; j++) {
            if (i == 0 && j >= player->boostsRemaining)
                continue;
            if (i == 1 && j >= (player->grenadeCount + 1) / 2)
                continue;
            if (i == 2 && j >= player->lives)
                continue;
            if (i == 3 && j >= player->missileCount)
                continue;
            nvgBeginPath(ctx);
            nvgMoveTo(ctx, g2[i][j][0][0], g2[i][j][0][1]);
            nvgLineTo(ctx, g2[i][j][1][0], g2[i][j][1][1]);
            nvgLineTo(ctx, g2[i][j][2][0], g2[i][j][2][1]);
            nvgLineTo(ctx, g2[i][j][3][0], g2[i][j][3][1]);
            nvgClosePath(ctx);
            nvgFillColor(ctx, g2c[j]);
            nvgFill(ctx);
        }
    }
    if (itsGame->veryLongWait) {
        nvgBeginPath(ctx);
        nvgRect(ctx, g1X, gY, 8.0, 8.0);
        nvgFillColor(ctx, nvgRGBA(255, 255, 0, 255));
        nvgFill(ctx);
    }
    if (itsGame->longWait) {
        nvgBeginPath(ctx);
        nvgRect(ctx, g1X + 4, gY + 4, 8.0, 8.0);
        nvgFillColor(ctx, nvgRGBA(255, 192, 0, 255));
        nvgFill(ctx);
    }
    nvgFillColor(ctx, nvgRGBA(255, 255, 255, 255));
    nvgFontSize(ctx, fontsz_m);
    nvgTextAlign(ctx, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
    if(player->itsManager != NULL) {
        snprintf(scoreText, sizeof(scoreText), "%ld", itsGame->scores[player->itsManager->Slot()]);
        nvgText(ctx, g1X + 22.5, gY + full + 10.0, scoreText, NULL);
    }

    secs = timeTemp % 60;
    timeTemp /= 60;
    mins = timeTemp % 60;
    timeTemp /= 60;
    snprintf(timeText, sizeof(timeText), "%ld:%02d:%02d", timeTemp, mins, secs);
    nvgFillColor(ctx, nvgRGBA(255, 255, 255, 255));
    nvgTextAlign(ctx, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
    nvgText(ctx, g2X1 + 7.5, gY + full + 10.0, timeText, NULL);

    nvgEndFrame(ctx);
}
