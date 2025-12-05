/*
  Warnings:

  - You are about to drop the column `authExpiry` on the `Player` table. All the data in the column will be lost.
  - You are about to drop the column `authToken` on the `Player` table. All the data in the column will be lost.

*/
-- RedefineTables
PRAGMA defer_foreign_keys=ON;
PRAGMA foreign_keys=OFF;
CREATE TABLE "new_Player" (
    "user_id" TEXT NOT NULL PRIMARY KEY,
    "username" TEXT NOT NULL,
    "email" TEXT NOT NULL,
    "magicToken" TEXT,
    "tokenExpiry" DATETIME,
    "sessionToken" TEXT,
    "wins" INTEGER NOT NULL DEFAULT 0,
    "goals" INTEGER NOT NULL DEFAULT 0,
    "games" INTEGER NOT NULL DEFAULT 0,
    "losses" INTEGER NOT NULL DEFAULT 0,
    "ratio" REAL,
    "role" TEXT NOT NULL DEFAULT 'player'
);
INSERT INTO "new_Player" ("email", "games", "goals", "losses", "magicToken", "ratio", "role", "tokenExpiry", "user_id", "username", "wins") SELECT "email", "games", "goals", "losses", "magicToken", "ratio", "role", "tokenExpiry", "user_id", "username", "wins" FROM "Player";
DROP TABLE "Player";
ALTER TABLE "new_Player" RENAME TO "Player";
CREATE UNIQUE INDEX "Player_username_key" ON "Player"("username");
CREATE UNIQUE INDEX "Player_email_key" ON "Player"("email");
CREATE UNIQUE INDEX "Player_magicToken_key" ON "Player"("magicToken");
CREATE UNIQUE INDEX "Player_sessionToken_key" ON "Player"("sessionToken");
PRAGMA foreign_keys=ON;
PRAGMA defer_foreign_keys=OFF;
