diff --git a/ai.cpp b/ai.cpp
index b43ab844b01ad5831fcf81dbcd6dbeea85c64d22..ceb74c9b0759251ce7db896e987fb7cb8d6af060 100644
--- a/ai.cpp
+++ b/ai.cpp
@@ -1,36 +1,37 @@
 #include "ai.h"
 #include <vector>
 #include <algorithm>
+#include <cstdlib>
 #include <QElapsedTimer>
 #include <QDebug>
 
 int AI::nodeCount = 0;
 
-const Piece::PieceColor aiColor = Piece::BLACK; //ai  !
+const Piece::PieceColor aiColor = Piece::BLACK; //ai ¼³Á¤Àº °ËÁ¤!
 
-const int AI::pawnPST[8][8] = { //pst ̺ °ǵ  ̷ Ǿִ  Ű ó ٰ Ʈʹ Ƽ ϴϱ ̹ ִ
+const int AI::pawnPST[8][8] = { //pst Å×ÀÌºí °¡Á®¿Â°Çµ¥ Æù¸¸ ÀÌ·¸°Ô µÇ¾îÀÖ´Â ÀÌÀ¯´Â ÅÇÅ°·Î Ã³À½¿¡ ¸ÂÃâ·Á´Ù°¡ ³ªÀÌÆ®ºÎÅÍ´Â ±ÍÂú¾Æ¼­ º¹ºÙÇÏ´Ï±î ÀÌ¹Ì ¸ÂÃçÁ®ÀÖ´õ¶ó°í¿ä
     {9000,  9000,   9000,   9000,   9000,   9000,   9000,   9000},
     {200,   200,    200,    200,    200,    200,    200,    200},
     {100,   100,    100,    100,    100,    100,    100,    100},
     {40,    40,     90,     100,    100,    90,     40,     40},
     {20,    20,     20,     100,    150,    20,     20,     20},
     {2,     4,      0,      15,     4,      0,      4,      2},
     {-10,   -10,    -10,    -20,    -35,    -10,    -10,    -10},
     {0,     0,      0,      0,      0,      0,      0,      0}
 };
 
 const int AI::knightPST[8][8] = {
     {-20, -80, -60, -60, -60, -60, -80, -20},
     {-80, -40,   0,   0,   0,   0, -40, -80},
     {-60,   0,  20,  30,  30,  20,   0, -60},
     {-60,  10,  30,  40,  40,  30,  10, -60},
     {-60,   0,  30,  40,  40,  30,   0, -60},
     {-60,  10,  20,  30,  30,  30,   1, -60},
     {-80, -40,   0,  10,  10,   0,  -4, -80},
     {-20, -80, -60, -60, -60, -60, -80, -20},
 };
 
 const int AI::bishopPST[8][8] = {
     {-40, -20, -20, -20, -20, -20, -20, -40},
     {-20,   0,   0,   0,   0,   0,   0, -20},
     {-20,   0,  10,  20,  20,  10,   0, -20},
@@ -41,186 +42,231 @@ const int AI::bishopPST[8][8] = {
     {-40, -20, -20, -20, -20, -20, -20, -40}
 };
 
 const int AI::rookPST[8][8] = {
     {0,  0,  0,  0,  0,  0,  0,   0},
     {10, 20, 20, 20, 20, 20, 20,  10},
     {-10,  0,  0,  0,  0,  0,  0, -10},
     {-10,  0,  0,  0,  0,  0,  0, -10},
     {-10,  0,  0,  0,  0,  0,  0, -10},
     {-10,  0,  0,  0,  0,  0,  0, -10},
     {-10,  0,  0,  0,  0,  0,  0, -10},
     {-30, 30, 40, 10, 10,  0,  0, -30}
 };
 
 const int AI::queenPST[8][8] = {
     {-40, -20, -20, -10, -10, -20, -20, -40},
     {-20,   0,   0,   0,   0,   0,   0, -20},
     {-20,   0,  10,  10,  10,  10,   0, -20},
     {-10,   0,  10,  10,  10,  10,   0, -10},
     {0,   0,  10,  10,  10,  10,   0, -10},
     {-20,  10,  10,  10,  10,  10,   0, -20},
     {-20,   0,  10,   0,   0,   0,   0, -20},
     {-40, -20, -20, -10, -10, -20, -20, -40}
 };
 
-const int AI::kingPST[8][8] = { //ŷ ߿ Ĺ ̺  Űƿ
+const int AI::kingPST[8][8] = { //Å·Àº ³ªÁß¿¡ ÈÄ¹ÝÀü¿ë Å×ÀÌºí µû·Î ¸¸µé°Å°°¾Æ¿ä
     {-60, -80, -80, -2, -20, -80, -80, -60},
     {-60, -80, -80, -2, -20, -80, -80, -60},
     {-60, -80, -80, -2, -20, -80, -80, -60},
     {-60, -80, -80, -2, -20, -80, -80, -60},
     {-40, -60, -60, -8, -80, -60, -60, -40},
     {-20, -40, -40, -40,-40, -40, -40, -20},
     {40,  40,   0,   0,  0,   0,  40,  40},
     {40,  60,  20,   0,  0,  20,  60,  40}
 };
 
 int AI::eval(const Game& game)
 {
-    int totalScore = 0; // 
+    int totalScore = 0; //ÃÑ Á¡¼ö¿¡¿ä
 
     for(int x = 0; x < 8; ++x)
     {
         for(int y = 0; y < 8; ++y)
         {
-            Piece* p = game.getPiece(x,y); //带 ȸϸ鼭 ⹰  Ϳ
+            Piece* p = game.getPiece(x,y); //º¸µå¸¦ ¼øÈ¸ÇÏ¸é¼­ ±â¹°µé Á¤º¸¸¦ °¡Á®¿Í¿ä
 
-            if(p != nullptr) //ǥ ⹰ ִ 
+            if(p != nullptr) //ÁÂÇ¥¿¡ ±â¹°ÀÌ ÀÖ´Â °æ¿ì
             {
                 int pieceScore = 0;
 
-                pieceScore += p->getValue(); //⹰  ϰ
+                pieceScore += p->getValue(); //±â¹°ÀÇ Á¡¼ö¸¦ ´õÇÏ°í¿ä
 
-                int pstY = (p->getColor() == Piece::WHITE) ? (7 - y) : y; // п  yǥ !
+                int pstY = (p->getColor() == Piece::WHITE) ? (7 - y) : y; //Èæ¹é ±¸ºÐ¿¡ µû¶ó yÁÂÇ¥ ¹ÝÀü!
 
-                switch (p->getType()) //̰ 鼭 Ѱǵ PST ̺ ִ ״  ,  ̷ ǥ y, x ǳ׿ 
-                { //ƹư  ġ    ϰԵ
+                switch (p->getType()) //ÀÌ°Å ¸¸µé¸é¼­ »ý°¢ÇÑ°Çµ¥¿ä PST Å×ÀÌºí¿¡ ÀÖ´ø°Å ±×´ë·Î ¹ÚÀ¸¸é ¸î, ¸î ÀÌ·±½ÄÀ¸·Î Ç¥ÇöÇßÀ»¶§ y, x°¡ µÇ³×¿ä ÇãÇã
+                { //¾Æ¹«Æ° º¸µåÀÇ À§Ä¡¿¡ µû¶ó Á¡¼ö¸¦ ´Ù ´õÇÏ°ÔµÇÁÒ
                 case Piece::PAWN: pieceScore += pawnPST[pstY][x]; break;
                 case Piece::BISHOP: pieceScore += bishopPST[pstY][x]; break;
                 case Piece::KNIGHT: pieceScore += knightPST[pstY][x]; break;
                 case Piece::ROOK: pieceScore += rookPST[pstY][x]; break;
                 case Piece::QUEEN: pieceScore += queenPST[pstY][x]; break;
                 case Piece::KING: pieceScore += kingPST[pstY][x]; break;
                 default: break;
                 }
 
-                if(p->getColor() == aiColor) //ΰ̶    ϰ
+                if(p->getColor() == aiColor) //ÀÎ°øÁö´ÉÀÌ¶û °°Àº »öÀÎ °æ¿ì ´õÇÏ°í
                 {
                     totalScore += pieceScore;
                 }
-                else //ٸ  
+                else //´Ù¸£¸é Á¡¼ö¸¦ ±ðÁÒ
                 {
                     totalScore -= pieceScore;
                 }
             }
         }
     }
-    return totalScore; //׷ Ѱ ȯؿ
+    return totalScore; //±×·¸°Ô ´õÇÑ°É ¹ÝÈ¯ÇØ¿ä
 }
 
-int AI::alphabeta(Game& state, int depth, int alpha, int beta, bool maxing) //Ⱑ ĺŸ ġ!
-{// ai.hκп  ؿ
+int AI::scoreMove(const Game& state, const Move& move, Piece::PieceColor moverColor)
+{
+    // MVV-LVA ½ÃÅ©·¯·Î °ø°£°ú ÀÐÇÏ½Ã ¹ü³ªµå¸¦ Á÷¼±ÇÏ°í Á¦ÇÑÀ» ÀÎµ¦¾î´Ù
+    const Piece* movingPiece = state.getPiece(move.fromX, move.fromY);
+    const Piece* targetPiece = state.getPiece(move.toX, move.toY);
+
+    int captureScore = 0;
+    if (targetPiece)
+    {
+        captureScore = (targetPiece->getValue() * 10);
+        if (movingPiece)
+        {
+            captureScore -= movingPiece->getValue();
+        }
+    }
+
+    // »óÁö ÁÖµµ°¡ ³·À½°ú ÁöÁ¤¿¡ ÀÇÇØ °¡ÁöÄ¡°¡ ¸Å¸® ´Ù¸¥ ¼ö ÀÖµµ·Ï Á¦¼ö
+    int centerDistance = (3 - std::abs(3 - move.toX)) + (3 - std::abs(3 - move.toY));
+
+    // Æ¯º°ÇÑ ÆÄ¿ö Áõº¯À» Â÷·¹Æ®½ÃÄÑ ÁÖ±â »ç¿ë
+    int pawnPush = 0;
+    if (movingPiece && movingPiece->getType() == Piece::PAWN)
+    {
+        pawnPush = (moverColor == Piece::WHITE) ? (move.fromY - move.toY) : (move.toY - move.fromY);
+    }
+
+    return captureScore * 100 + centerDistance * 5 + pawnPush;
+}
+
+int AI::alphabeta(Game& state, int depth, int alpha, int beta, bool maxing) //¿©±â°¡ ¾ËÆÄº£Å¸ °¡ÁöÄ¡±â!
+{// ai.hºÎºÐ¿¡¼­ ±íÀÌ Á¶Àý°¡´ÉÇØ¿ä¤Ë
     nodeCount++;
 
-    if(depth == 0 || state.isGameOver()) //̰ 0̰ų  ̻  ⹰  
+    if(depth == 0 || state.isGameOver()) //±íÀÌ°¡ 0ÀÌ°Å³ª ´õ ÀÌ»ó ¿òÁ÷ÀÏ ±â¹°ÀÌ ¾ø´Â °æ¿ì
     {
-        return eval(state); //ȯ ؿ
+        return eval(state); //¹ÝÈ¯À» ÇØ¿ä
     }
 
     vector<Move> allMoves = state.generateMoves(state.getCurrentTurn());
 
-    if(maxing) //ִȭ ؾϴ 
+    auto orderMoves = [&](std::vector<Move>& moves, Piece::PieceColor moverColor)
     {
-        int maxEval = -20000; //ϴ ΰ   Ѵ뿴 ׳ ̷ صѰԿ
+        std::sort(moves.begin(), moves.end(), [&](const Move& a, const Move& b)
+        {
+            return scoreMove(state, a, moverColor) > scoreMove(state, b, moverColor);
+        });
+    };
+
+    Piece::PieceColor moverColor = state.getCurrentTurn();
+    orderMoves(allMoves, moverColor);
 
-        for(int i = 0; i < allMoves.size(); ++i) // ϼִ   ȿ
+    if(maxing) //ÃÖ´ëÈ­¸¦ ÇØ¾ßÇÏ´Â °æ¿ì
+    {
+        int maxEval = -20000; //ÀÏ´Ü ÀÎ°øÁö´É ¼ö¾÷¿¡¼­´Â À½ÀÇ ¹«ÇÑ´ë¿´Áö¸¸ ±×³É ÀÌ·¸°Ô ÇØµÑ°Ô¿ä
+
+        for(int i = 0; i < allMoves.size(); ++i) //¸ðµç ¿òÁ÷ÀÏ¼öÀÖ´Â ±× °æ¿ì ¾È¿¡¼­
         {
-            Move move = allMoves[i]; //߿ ϳ Ϳ
+            Move move = allMoves[i]; //±×Áß¿¡ ÇÏ³ª¸¦ µé°í¿Í¿ä
 
             Piece* captured = state.makeMove(move);
-            int evalScore = alphabeta(state, depth - 1, alpha, beta, false); //⼭  ȣ ؿ maxing true falseĿ   ΰɿ ô  Ʈݾƿ ư 
+            int evalScore = alphabeta(state, depth - 1, alpha, beta, false); //¿©±â¼­ Àç±ÍÀûÀ¸·Î È£ÃâÀ» ÇØ¿ä maxingÀÌ true³Ä false³Ä¿¡ µû¶ó ±× ÀÎ°øÁö´É¿¡¼­ ºÃ´ø ±× Æ®¸®ÀÖÀÝ¾Æ¿ä ¹ø°¥¾Æ°¡°Ô µÇÁÒ
 
             state.unmakeMove(move, captured);
 
             maxEval = max(maxEval, evalScore);
             alpha = max(alpha, evalScore);
 
             if(beta <= alpha)
             {
-                break; //ġ;
+                break; //°¡ÁöÄ¡¤¡;
             }
         }
         return maxEval;
     }
-    else //⵵  ̷
+    else //¿©±âµµ ¸¶Âù°¡Áö·Î ÀÌ·ç¾îÁöÁÒ
     {
         int minEval = 20000;
 
         for(int i = 0; i < allMoves.size(); ++i)
         {
             Move move = allMoves[i];
 
             Piece* captured = state.makeMove(move);
             int evalScore = alphabeta(state, depth - 1, alpha, beta, true);
 
             state.unmakeMove(move, captured);
 
             minEval = min(minEval, evalScore);
             beta = min(beta, evalScore);
 
             if(beta <= alpha)
             {
                 break;
             }
         }
         return minEval;
     }
 }
 
 Move AI::findBestMove(const Game& game)
 {
-    if(game.getCurrentTurn() != aiColor) // ai ƴҶ ׳ ؿ
+    if(game.getCurrentTurn() != aiColor) // ai°¡ ¾Æ´Ò¶§ ±×³É ¹«½ÃÇØ¿ä
     {
         return Move();
     }
 
     Move bestMove;
 
     int maxEval = -20000;
 
     QElapsedTimer timer;
     timer.start();
     nodeCount = 0;
 
-    Game rootGame = game; //   ǵ ʱ 纻 
+    Game rootGame = game; //±¸Á¶ º¯°æÀ¸·Î ¿øº»À» °ÇµéÁö ¾Ê±âÀ§ÇØ º¹»çº» »ý¼º
     vector<Move> allMoves = rootGame.generateMoves(aiColor);
 
+    std::sort(allMoves.begin(), allMoves.end(), [&](const Move& a, const Move& b)
+    {
+        return scoreMove(rootGame, a, aiColor) > scoreMove(rootGame, b, aiColor);
+    });
+
     if(allMoves.empty())
     {
         return Move();
     }
 
     for(int i = 0; i < allMoves.size(); ++i)
     {
         Move move = allMoves[i];
 
         Piece* captured = rootGame.makeMove(move);
 
         int evalScore = alphabeta(rootGame, searchDepth - 1, -20000, 20000, false);
 
         rootGame.unmakeMove(move, captured);
 
         if(evalScore > maxEval)
         {
             maxEval = evalScore;
             bestMove = move;
         }
     }
 
     qDebug() << "========================================";
     qDebug() << "AI Search Depth:" << searchDepth;
-    qDebug() << "Time Elapsed:" << timer.elapsed() << "ms"; // ɸ ð (и)
-    qDebug() << "Nodes Visited:" << nodeCount;            // 湮  
+    qDebug() << "Time Elapsed:" << timer.elapsed() << "ms"; // °É¸° ½Ã°£ (¹Ð¸®ÃÊ)
+    qDebug() << "Nodes Visited:" << nodeCount;            // ¹æ¹®ÇÑ ³ëµå ¼ö
     qDebug() << "========================================";
 
     return bestMove;
 }
