/*
  Warnings:

  - A unique constraint covering the columns `[magicToken]` on the table `Player` will be added. If there are existing duplicate values, this will fail.

*/
-- AlterTable
ALTER TABLE "Player" ADD COLUMN "magicToken" TEXT;
ALTER TABLE "Player" ADD COLUMN "tokenExpiry" DATETIME;

-- CreateIndex
CREATE UNIQUE INDEX "Player_magicToken_key" ON "Player"("magicToken");
