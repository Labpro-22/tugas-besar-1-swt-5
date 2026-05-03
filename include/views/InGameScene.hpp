#ifndef IN_GAME_SCENE_HPP
#define IN_GAME_SCENE_HPP

#include <string>
#include <vector>
#include "Button.hpp"
#include "Scene.hpp"
#include "TextField.hpp"

class PropertyTile;
class TradeToPlayer;

class InGameScene : public Scene {
public:
    InGameScene(SceneManager* sm, GameManager* gm, AccountManager* am);
    void update() override;
    void draw()   override;
    void onEnter() override;
private:
    std::vector<Button> actionButtons;
    Button closeOverlayBtn;
    Button backToMenuBtn;
    Button saveConfirmButton;
    Button saveCancelButton;
    Button diceRollButton;
    Button diceManualButton;
    Button diceManualConfirmButton;
    Button diceBackButton;
    Button diceCancelButton;
    Button openLogButton;
    Button closeLogButton;
    Button closeCardButton;
    Button useCardButton;
    Button discardCardButton;
    Button cancelLassoTargetButton;
    std::vector<Button> lassoTargetButtons;
    Button auctionBidButton;
    Button auctionPassButton;
    Button auctionCloseButton;
    Button tradeSendButton;
    Button tradeCancelButton;
    Button tradeAcceptButton;
    Button tradeRejectButton;
    TextField savePathField;
    TextField diceOneField;
    TextField diceTwoField;
    TextField auctionBidField;
    TextField tradeTargetField;
    TextField tradeOfferPropsField;
    TextField tradeOfferMoneyField;
    TextField tradeRequestPropsField;
    TextField tradeRequestMoneyField;
    
    std::vector<Rectangle> tileRects;
    std::vector<Vector2> tokenPos;
    std::vector<float>   tokenPhase;
    float sceneTime;
    int selectedTile;
    
    bool overlayOpen;
    std::string overlayTitle;
    std::vector<std::string> overlayLines;
    std::string overlayFooter;
    float overlayVis;
    
    bool showSaveModal;
    std::string saveError;
    float saveModalVis;

    bool showDiceModal;
    bool diceManualMode;
    std::string diceError;
    float diceModalVis;

    bool showLogModal;
    float logModalVis;

    bool showCardModal;
    bool cardOverflowMode;
    int selectedCardIndex;
    std::string cardError;
    float cardModalVis;

    bool showLassoTargetModal;
    std::vector<int> lassoTargetPlayerIds;
    int pendingLassoCardIndex;
    float lassoTargetModalVis;

    bool propertyDecisionPending;
    bool propertyDecisionResolved;
    PropertyTile* pendingProperty;
    bool showAuctionModal;
    bool auctionNoticeMode;
    std::string auctionError;
    std::vector<std::string> auctionNoticeLines;
    float auctionModalVis;

    bool showTradeModal;
    bool tradeResponseMode;
    std::string tradeError;
    TradeToPlayer* pendingTrade;
    float tradeModalVis;

    void layoutUi(Rectangle sr, Rectangle& br, Rectangle& sb);
    Rectangle getTileRect(const Rectangle& br, int idx) const;
    Vector2   getTileCenter(const Rectangle& br, int idx) const;
    void updateAnimations(const Rectangle& br);
    void drawBackground(Rectangle sr);
    void drawHeader(Rectangle sr);
    void drawBoard(const Rectangle& br);
    void drawCenterPanel(const Rectangle& br);
    void drawSidebar(const Rectangle& sb);
    void drawOverlay(Rectangle sr);
    void showOverlay(
        const std::string& title,
        const std::vector<std::string>& lines,
        const std::string& footer = ""
    );

    void drawSaveModal(Rectangle sr);
    void drawDiceModal(Rectangle sr);
    void drawLogModal(Rectangle sr);
    void drawCardModal(Rectangle sr);
    void drawLassoTargetModal(Rectangle sr);
    void drawAuctionModal(Rectangle sr);
    void openCardModal(bool overflowMode);
    void onUseSelectedCard();
    void onDiscardSelectedCard();
    void openLassoTargetModal(const std::vector<int>& targetPlayerIds);
    void onLassoTargetSelected(int targetPlayerId);
    void drawTradeModal(Rectangle sr);
    void onSaveGame();
    void rollDiceAndShowResult();
    void onManualDiceSubmit();
    void refreshPropertyDecisionState();
    bool hasBlockingPropertyDecision() const;
    void startAuctionForProperty(PropertyTile* property, const std::string& reason = "");
    void onAuctionBid();
    void onAuctionPass();
    void finishAuctionNotice();
    void openTradeProposal();
    void onTradeSubmit();
    void onTradeAccept();
    void onTradeReject();
};

#endif
