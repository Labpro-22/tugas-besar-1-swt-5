#include "../../include/views/InGameScene.hpp"
#include "../../include/views/SceneManager.hpp"
#include "../../include/core/GameManager.hpp"
#include "../../include/core/Game.hpp"
#include "../../include/models/Player.hpp"
#include "../../include/models/AbilityCard.hpp"
#include "../../include/utils/Board.hpp"
#include "../../include/utils/Tile.hpp"
#include "../../include/utils/PropertyTile.hpp"
#include "../../include/utils/StreetTile.hpp"
#include "../../include/utils/RailroadTile.hpp"
#include "../../include/utils/UtilityTile.hpp"
#include "../../include/utils/LogEntry.hpp"
#include "../../include/core/TurnManager.hpp"
#include "../../include/utils/Logger.hpp"
#include "raylib.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <string>
#include <vector>

namespace {
const Color kBgA{230,245,210,255}, kBgB{255,248,195,255};
const Color kPanel{245,255,230,230}, kPanelBorder{160,200,100,180};
const Color kBoardSurf{240,248,220,255}, kCenterSurf{225,242,200,255};
const Color kText{45,75,15,255}, kSubtext{90,130,45,255};
const Color kAccent{255,190,30,255}, kAccentAlt{80,175,50,255};
const Color kDanger{210,70,50,255};
const std::array<Color,4> kTok = {Color{255,190,30,255},{80,175,50,255},{255,120,60,255},{60,160,220,255}};

Color groupCol(const std::string& g) {
    if(g=="COKLAT")return{135,86,58,255}; if(g=="BIRU_MUDA")return{124,215,255,255};
    if(g=="MERAH_MUDA")return{245,120,182,255}; if(g=="ORANGE")return{244,154,74,255};
    if(g=="MERAH")return{228,77,75,255}; if(g=="KUNING")return{241,213,81,255};
    if(g=="HIJAU")return{88,191,120,255}; if(g=="BIRU_TUA")return{65,92,202,255};
    if(g=="ABU"||g=="ABU_ABU")return{149,158,176,255};
    return{200,210,185,255};
}
float ease(float c,float t,float r){return c+(t-c)*std::max(0.f,std::min(r,1.f));}

void drawSmallFlower(float cx,float cy,float r,float a,float al){
    for(int i=0;i<5;++i){float an=a+i*(6.283f/5.f);
        DrawCircle(int(cx+r*1.3f*cosf(an)),int(cy+r*1.3f*sinf(an)),r,Fade(kAccent,al));}
    DrawCircle(int(cx),int(cy),r*.6f,Fade({255,160,30,255},al));
}
}

InGameScene::InGameScene(SceneManager* sm, GameManager* gm, AccountManager* am)
    : Scene(sm,gm,am),
      closeOverlayBtn("X",kDanger,{255,255,255,255}),
      backToMenuBtn("Menu",kSubtext,{255,255,255,255}),
      sceneTime(0), selectedTile(0), overlayOpen(false), overlayVis(0) {

    struct Spec { const char* label; std::function<void()> fn; };
    std::vector<Spec> specs = {
        {"Lempar Dadu",[this](){
            Game* g = gameManager->getCurrentGame(); if(!g)return;
            auto r = g->getDice().roll(); int d1=r.first,d2=r.second;
            int idx = g->getTurnManager().getCurrentPlayerIndex();
            Player& p = g->getPlayer(idx);
            bool passGo = false;
            if(g->getBoard().size()==0)return;
            int np = g->getBoard().calculateNewPosition(p.getPosition(),d1+d2,passGo);
            p.moveTo(np);
            if(passGo){int sal=g->getConfig().getSpecialConfig(GO_SALARY);p.receive(sal);
                g->getLogger().log(g->getTurnManager().getCurrentTurn(),p.getUsername(),"GO","M"+std::to_string(sal));}
            g->handleLanding(p);
            g->getLogger().log(g->getTurnManager().getCurrentTurn(),p.getUsername(),"DADU",
                std::to_string(d1)+"+"+std::to_string(d2)+"="+std::to_string(d1+d2));
        }},
        {"Info Petak",[this](){
            Game* g = gameManager->getCurrentGame(); if(!g)return;
            Tile* t = g->getBoard().getTileByIndex(selectedTile);
            if(!t)return;
            std::vector<std::string> lines = {"Kode: "+t->getCode(),"Nama: "+t->getName()};
            PropertyTile* pt = dynamic_cast<PropertyTile*>(t);
            if(pt){lines.push_back("Harga: M"+std::to_string(pt->getLandPrice()));
                lines.push_back("Pemilik: "+(pt->getOwner()?pt->getOwner()->getUsername():"BANK"));
                StreetTile* st=dynamic_cast<StreetTile*>(pt);
                if(st){lines.push_back("Grup: "+st->getColorGroup());
                    lines.push_back("Rumah: "+std::to_string(st->getHouseCount())+(st->hasHotelBuilt()?" (Hotel)":""));}}
            showOverlay(t->getName(),lines);
        }},
        {"Properti",[this](){
            Game* g = gameManager->getCurrentGame(); if(!g)return;
            int idx=g->getTurnManager().getCurrentPlayerIndex();
            Player& p = g->getPlayer(idx);
            std::vector<std::string> lines = {"Pemilik: "+p.getUsername()};
            for(PropertyTile* pt:p.getOwnedProperties()) lines.push_back(pt->getCode()+" - "+pt->getName()+(pt->isMortgaged()?" [GADAI]":""));
            if(lines.size()==1) lines.push_back("(tidak ada)");
            showOverlay("Properti "+p.getUsername(),lines);
        }},
        {"Kartu",[this](){
            Game* g = gameManager->getCurrentGame(); if(!g)return;
            int idx=g->getTurnManager().getCurrentPlayerIndex();
            Player& p = g->getPlayer(idx);
            std::vector<std::string> lines;
            for(const auto& c:p.getHandCards()) lines.push_back(c->getName()+" - "+c->getDescription());
            if(lines.empty()) lines.push_back("(tidak ada kartu)");
            showOverlay("Kartu Kemampuan",lines);
        }},
        {"Akhir Giliran",[this](){
            Game* g = gameManager->getCurrentGame(); if(!g)return;
            g->endTurn();
        }},
    };
    for(auto& s:specs){Button b(s.label,kAccentAlt,{255,255,255,255});b.setOnClick(s.fn);actionButtons.push_back(b);}
    closeOverlayBtn.setOnClick([this](){overlayOpen=false;});
    backToMenuBtn.setOnClick([this](){sceneManager->setScene(SceneType::MainMenu);});
}

void InGameScene::onEnter(){sceneTime=0;overlayVis=0;overlayOpen=false;tokenPos.clear();tokenPhase.clear();selectedTile=0;}
void InGameScene::showOverlay(const std::string& t,const std::vector<std::string>& l){overlayTitle=t;overlayLines=l;overlayOpen=true;}

void InGameScene::layoutUi(Rectangle sr,Rectangle& br,Rectangle& sb){
    float sm=20,tm=88,gap=16;
    float sw=std::max(330.f,std::min(sr.width*.28f,380.f));
    float bs=std::min(sr.height-tm-24,sr.width-sw-gap-sm*2);
    br={sm,tm,bs,bs}; sb={br.x+br.width+gap,tm,sw,bs};
}

Rectangle InGameScene::getTileRect(const Rectangle& br,int i) const {
    float c=br.width/11.f;
    if(i>=0&&i<=10)return{br.x+br.width-(i+1.f)*c,br.y+br.height-c,c,c};
    if(i>=11&&i<=19)return{br.x,br.y+br.height-(i-10+1.f)*c,c,c};
    if(i>=20&&i<=30)return{br.x+(i-20.f)*c,br.y,c,c};
    return{br.x+br.width-c,br.y+(i-30.f)*c,c,c};
}
Vector2 InGameScene::getTileCenter(const Rectangle& br,int i) const{auto r=getTileRect(br,i);return{r.x+r.width*.5f,r.y+r.height*.5f};}

void InGameScene::updateAnimations(const Rectangle& br){
    Game* g = gameManager->getCurrentGame(); if(!g)return;
    size_t pc = g->getPlayers().size();
    if(tokenPos.size()!=pc){tokenPos.assign(pc,{0,0});tokenPhase.assign(pc,0);
        for(size_t i=0;i<pc;++i){tokenPos[i]=getTileCenter(br,g->getPlayers()[i].getPosition());tokenPhase[i]=i*1.37f;}}
    float dt=GetFrameTime();
    for(size_t i=0;i<pc;++i){
        Vector2 tgt=getTileCenter(br,g->getPlayers()[i].getPosition());
        tokenPos[i].x=ease(tokenPos[i].x,tgt.x,dt*6);tokenPos[i].y=ease(tokenPos[i].y,tgt.y,dt*6);
        tokenPhase[i]+=dt*(1.1f+i*.12f);}
    overlayVis=ease(overlayVis,overlayOpen?1.f:0.f,dt*8);
}

void InGameScene::update(){
    sceneTime+=GetFrameTime();
    Rectangle sr{0,0,(float)GetScreenWidth(),(float)GetScreenHeight()},br{},sb{};
    layoutUi(sr,br,sb);
    tileRects.clear();
    Game* g = gameManager->getCurrentGame();
    int tileCount = g ? g->getBoard().size() : 0;
    for(int i=0;i<tileCount;++i) tileRects.push_back(getTileRect(br,i));
    updateAnimations(br);
    if(IsKeyPressed(KEY_ESCAPE)){if(overlayOpen)overlayOpen=false;else{sceneManager->setScene(SceneType::MainMenu);return;}}
    if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){Vector2 m=GetMousePosition();
        for(size_t i=0;i<tileRects.size();++i) if(CheckCollisionPointRec(m,tileRects[i])){selectedTile=int(i);break;}}
    backToMenuBtn.setBoundary({sr.width-130,22,106,42});backToMenuBtn.update();
    float bw=(sb.width-14)*.5f,bh=44,sy=sb.y+238;
    for(size_t i=0;i<actionButtons.size();++i){
        int r=int(i)/2,c=int(i)%2;
        actionButtons[i].setBoundary({sb.x+c*(bw+14),sy+r*54,bw,bh});actionButtons[i].update();}
    if(overlayVis>.01f){closeOverlayBtn.setBoundary({sr.width*.5f+268,sr.height*.5f-240,50,38});closeOverlayBtn.update();}
}

void InGameScene::drawBackground(Rectangle sr){
    DrawRectangleGradientV(0,0,int(sr.width),int(sr.height),kBgA,kBgB);
    drawSmallFlower(40,40,18,sceneTime*.3f,.3f);
    drawSmallFlower(sr.width-50,50,14,sceneTime*.35f+1,.25f);
    drawSmallFlower(30,sr.height-50,12,sceneTime*.4f+2,.25f);
    drawSmallFlower(sr.width-40,sr.height-40,16,sceneTime*.3f+.5f,.25f);
    for(int i=0;i<10;++i){float x=fmodf(60+i*143+sceneTime*(8+i*.4f),sr.width+50)-25;
        float y=40+fmodf(i*73+sceneTime*(5+i*.25f),sr.height-80);
        DrawEllipse(int(x),int(y),4+i%3,2+i%2,Fade(kAccentAlt,.15f));}
}

void InGameScene::drawHeader(Rectangle sr){
    Game* g = gameManager->getCurrentGame();
    DrawText("Nimonspoli",22,16,34,kText);
    if(g){
        int ci=g->getTurnManager().getCurrentPlayerIndex();
        std::string status = (ci>=0&&ci<(int)g->getPlayers().size()) ? "Giliran: "+g->getPlayers()[ci].getUsername() : "";
        DrawText(status.c_str(),22,52,20,kSubtext);
        Rectangle badge{380,18,240,46};
        DrawRectangleRounded(badge,.28f,8,Fade(kAccent,.2f));
        DrawRectangleRoundedLinesEx(badge,.28f,8,1.5f,Fade(kPanelBorder,.8f));
        std::string tt="Turn "+std::to_string(g->getTurnManager().getCurrentTurn())+" / "+std::to_string(g->getTurnManager().getMaxTurn());
        DrawText(tt.c_str(),int(badge.x+20),int(badge.y+12),22,kText);
    }
    backToMenuBtn.draw();
}

void InGameScene::drawCenterPanel(const Rectangle& br){
    float c=br.width/11.f;
    Rectangle ctr{br.x+c,br.y+c,br.width-2*c,br.height-2*c};
    DrawRectangleRounded(ctr,.05f,10,kCenterSurf);
    DrawRectangleRoundedLinesEx(ctr,.05f,10,3,Fade(kAccentAlt,.6f));
    Rectangle rib{ctr.x+ctr.width*.25f,ctr.y+ctr.height*.42f,ctr.width*.5f,68};
    DrawRectanglePro({rib.x+rib.width*.5f,rib.y+rib.height*.5f,rib.width,rib.height},{rib.width*.5f,rib.height*.5f},-22,kAccent);
    DrawText("NIMONSPOLI",int(ctr.x+ctr.width*.27f),int(ctr.y+ctr.height*.43f),38,kText);
    drawSmallFlower(ctr.x+ctr.width*.5f,ctr.y+ctr.height*.72f,20,sceneTime*.6f,.4f);
}

void InGameScene::drawBoard(const Rectangle& br){
    Game* g = gameManager->getCurrentGame(); if(!g)return;
    DrawRectangleRounded({br.x+5,br.y+7,br.width,br.height},.04f,10,Fade(kText,.08f));
    DrawRectangleRounded(br,.04f,10,kBoardSurf);
    DrawRectangleRoundedLinesEx(br,.04f,10,3,Fade(kAccentAlt,.7f));

    if(g->getBoard().size()==0) { drawCenterPanel(br); return; }
    const auto& tiles = g->getBoard().getTiles();
    for(size_t i=0;i<tiles.size()&&i<tileRects.size();++i){
        Tile* tile=tiles[i]; if(!tile)continue;
        Rectangle r=tileRects[i];
        bool sel=(int(i)==selectedTile);
        DrawRectangleRec(r,kBoardSurf);
        DrawRectangleLinesEx(r,1,Fade(kPanelBorder,.5f));

        // Color band
        std::string cg="DEFAULT";
        StreetTile* st=dynamic_cast<StreetTile*>(tile);
        if(st) cg=st->getColorGroup();
        Rectangle band=r;
        if(i<=10){band.height=16;band.y=r.y+r.height-band.height;}
        else if(i<=19){band.width=16;}
        else if(i<=30){band.height=16;}
        else{band.width=16;band.x=r.x+r.width-band.width;}
        DrawRectangleRec(band,groupCol(cg));

        if(sel){float pu=.5f+.5f*sinf(sceneTime*3);DrawRectangleLinesEx(r,3+pu*2,kAccent);}
        DrawText(tile->getCode().c_str(),int(r.x+5),int(r.y+5),16,kText);
        DrawText(tile->getName().c_str(),int(r.x+5),int(r.y+25),10,kSubtext);

        PropertyTile* pt=dynamic_cast<PropertyTile*>(tile);
        if(pt&&pt->getOwner()){
            int oi=0; for(auto& p:g->getPlayers()){if(&p==pt->getOwner())break;oi++;}
            DrawCircle(int(r.x+r.width-14),int(r.y+14),7,kTok[oi%4]);}
        if(pt&&pt->isMortgaged()) DrawText("G",int(r.x+r.width-22),int(r.y+26),14,kDanger);
        if(st){for(int h=0;h<st->getHouseCount()&&h<4;++h) DrawRectangle(int(r.x+7+h*11),int(r.y+r.height-34),8,8,kAccentAlt);
            if(st->hasHotelBuilt()) DrawRectangle(int(r.x+7),int(r.y+r.height-34),16,10,kDanger);}
        if(pt&&pt->getFestivalMultiplier()>1){std::string fs="x"+std::to_string(pt->getFestivalMultiplier());
            DrawText(fs.c_str(),int(r.x+r.width-26),int(r.y+r.height-22),13,{180,50,200,255});}
    }

    drawCenterPanel(br);

    // Tokens
    for(size_t i=0;i<tokenPos.size();++i){
        float bob=sinf(tokenPhase[i]*2)*4;
        Vector2 pos{tokenPos[i].x,tokenPos[i].y+bob};
        Color tint=kTok[i%4];
        DrawCircleLines(int(pos.x),int(pos.y),14+sinf(tokenPhase[i]*2.5f)*2,Fade(tint,.3f));
        DrawCircle(int(pos.x),int(pos.y),10,tint);
        DrawCircleLines(int(pos.x),int(pos.y),10,Fade(WHITE,.5f));
        DrawText(std::to_string(int(i)+1).c_str(),int(pos.x-4),int(pos.y-6),12,kText);
    }
}

void InGameScene::drawSidebar(const Rectangle& sb){
    Game* g = gameManager->getCurrentGame(); if(!g||g->getPlayers().empty())return;

    DrawRectangleRounded({sb.x+4,sb.y+7,sb.width,sb.height},.04f,10,Fade(kText,.07f));
    DrawRectangleRounded(sb,.04f,10,Fade(kPanel,.97f));
    DrawRectangleRoundedLinesEx(sb,.04f,10,2,Fade(kPanelBorder,.8f));
    drawSmallFlower(sb.x+sb.width-22,sb.y+22,9,sceneTime*.4f,.35f);

    int ci=g->getTurnManager().getCurrentPlayerIndex();
    if(ci<0||ci>=(int)g->getPlayers().size())return;
    Player& cur=g->getPlayer(ci);
    DrawText("Giliran Sekarang",int(sb.x+16),int(sb.y+16),24,kText);
    DrawCircle(int(sb.x+28),int(sb.y+66),11,kTok[ci%4]);
    DrawText(cur.getUsername().c_str(),int(sb.x+50),int(sb.y+52),26,kText);
    DrawText(("M"+std::to_string(cur.getMoney())).c_str(),int(sb.x+16),int(sb.y+88),22,kAccentAlt);

    if(g->getBoard().size()>0 && cur.getPosition()<g->getBoard().size()){
        Tile* posTile=g->getBoard().getTileByIndex(cur.getPosition());
        if(posTile) DrawText((posTile->getCode()+" - "+posTile->getName()).c_str(),int(sb.x+16),int(sb.y+116),18,kSubtext);
    }

    // Progress bar
    Rectangle wave{sb.x+16,sb.y+150,sb.width-32,16};
    DrawRectangleRounded(wave,.8f,10,Fade(kPanelBorder,.35f));
    float prog = g->getTurnManager().getMaxTurn()>0?(float)g->getTurnManager().getCurrentTurn()/g->getTurnManager().getMaxTurn():.5f;
    DrawRectangleRounded({wave.x,wave.y,wave.width*prog,wave.height},.8f,10,kAccent);

    // Action buttons
    for(Button& b:actionButtons) b.draw();

    // Selected tile
    Tile* sel = (g->getBoard().size()>0 && selectedTile<g->getBoard().size()) ? g->getBoard().getTileByIndex(selectedTile) : nullptr;
    if(sel){
        Rectangle ins{sb.x,sb.y+404,sb.width,106};
        DrawRectangleRounded({ins.x+8,ins.y,ins.width-16,ins.height},.14f,8,Fade(kAccent,.1f));
        DrawText("Petak Dipilih",int(ins.x+16),int(ins.y+8),22,kText);
        DrawText((sel->getCode()+" - "+sel->getName()).c_str(),int(ins.x+16),int(ins.y+38),20,kText);
    }

    // All players
    DrawText("Semua Pemain",int(sb.x+16),int(sb.y+524),22,kText);
    for(size_t i=0;i<g->getPlayers().size();++i){
        float py=sb.y+556+i*28;
        DrawCircle(int(sb.x+20),int(py+8),6,kTok[i%4]);
        Player& p=g->getPlayers()[i];
        std::string row=p.getUsername()+" M"+std::to_string(p.getMoney());
        if(p.isBankrupt()) row+=" [BANGKRUT]";
        else if(p.getStatus()==PlayerStatus::JAILED) row+=" [PENJARA]";
        DrawText(row.c_str(),int(sb.x+36),int(py),17,int(i)==ci?kText:kSubtext);
    }

    // Hand cards
    DrawText("Kartu Kemampuan",int(sb.x+16),int(sb.y+684),22,kText);
    const auto& hand=cur.getHandCards();
    for(size_t i=0;i<hand.size();++i){
        float sway=sinf(sceneTime*1.7f+i)*3;
        Rectangle card{sb.x+16+i*100,sb.y+716+sway,88,38};
        DrawRectangleRounded(card,.22f,8,Fade(kAccent,.25f));
        DrawRectangleRoundedLinesEx(card,.22f,8,1.5f,Fade(kPanelBorder,.7f));
        DrawText(hand[i]->getName().c_str(),int(card.x+7),int(card.y+10),14,kText);
    }

    // Log
    DrawText("Log Terbaru",int(sb.x+16),int(sb.y+776),22,kText);
    auto entries=g->getLogger().getEntries();
    int lc=std::min<int>(3,int(entries.size()));
    for(int i=0;i<lc;++i){
        const LogEntry& e=entries[size_t(i)];
        float ly=sb.y+808+i*40;
        DrawText(("[T"+std::to_string(e.getTurn())+"] "+e.getUsername()+" | "+e.getActionType()).c_str(),int(sb.x+16),int(ly),16,kText);
        DrawText(e.getDetail().c_str(),int(sb.x+16),int(ly+17),15,kSubtext);
    }
}

void InGameScene::drawOverlay(Rectangle sr){
    if(overlayVis<=.01f||!overlayOpen)return;
    DrawRectangle(0,0,GetScreenWidth(),GetScreenHeight(),Fade(kText,.38f*overlayVis));
    float rise=(1-overlayVis)*24;
    Rectangle p{sr.width*.5f-310,sr.height*.5f-252+rise,620,504};
    DrawRectangleRounded({p.x+5,p.y+9,p.width,p.height},.09f,10,Fade(kText,.12f*overlayVis));
    DrawRectangleRounded(p,.09f,10,Fade({250,255,235,255},overlayVis));
    DrawRectangleRoundedLinesEx(p,.09f,10,2.5f,Fade(kPanelBorder,overlayVis));
    drawSmallFlower(p.x+p.width-28,p.y+28,14,sceneTime*.5f,.5f*overlayVis);
    DrawText(overlayTitle.c_str(),int(p.x+22),int(p.y+20),32,kText);
    closeOverlayBtn.draw();
    float y=p.y+80;
    for(const auto& l:overlayLines){DrawText(l.c_str(),int(p.x+24),int(y),20,kSubtext);y+=32;if(y>p.y+p.height-40)break;}
}

void InGameScene::draw(){
    Rectangle sr{0,0,(float)GetScreenWidth(),(float)GetScreenHeight()},br{},sb{};
    layoutUi(sr,br,sb);
    drawBackground(sr); drawHeader(sr); drawBoard(br); drawSidebar(sb); drawOverlay(sr);
}
