/*
  Warnings:

  - A unique constraint covering the columns `[authToken]` on the table `Player` will be added. If there are existing duplicate values, this will fail.

*/
-- AlterTable
ALTER TABLE "Player" ADD COLUMN "authExpiry" DATETIME;
ALTER TABLE "Player" ADD COLUMN "authToken" TEXT;

-- CreateIndex
CREATE UNIQUE INDEX "Player_authToken_key" ON "Player"("authToken");
