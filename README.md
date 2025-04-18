# Soccer Robots CS (UTDesign EPICS)

<!-- markdownlint-disable-next-line MD033 -->
<details><summary><h2>Table of Contents</h2></summary>

- [Conceptual Overview](#conceptual-overview)
- [Functional Requirements](#functional-requirements)
- [Third Party Integrations](#third-party-integrations)
- [Tech Stack](#tech-stack)
- [Deployment Notes](#deployment-notes)
- [Migration Scripts](#migration-scripts)
- [Setting Up Development Environment](#setting-up-development-environment)

</details>

## Conceptual Overview

The Jonsson School UTDesign program serves students from UTDesign Capstone, EPICS, Makerspace and Startup. Our partner is Tim Givens, the UTDesign Studio Assistant Manager, who has the following mission:

*To bring UTD to the forefront of robotics competitions while collaborating with local high schools to get more kids engaged in robotics and STEM.*

Soccer Robots is an interactive live-fed soccer game between two robots where players can control their robots remotely over the internet. The Soccer Robots team is split into CS and Engineering teams. We are Soccer Robots CS, tasked with

- Handle serving webpage
- Handle user accounts through Auth0
- Integrate Twitch livestream
- Display game information such as time and scoreboard
- Implement queue system
- Set up and record matches
- Forward wasd inputs from players to Engineering team (Raspberry server)
- Communicate with Engineering team for robots' status

Our project not only caters to the entertainment needs of our users but also serves as a captivating demonstration of UTDesign's capabilities!

## Repository Branches Map
- main: Website / user side of project
- main_robot: Main branch for the robot side of the project
  - esp32_code: Development branch for robot side of the project
- raspberry_pi: Main branch for web/game server code. A raspberry pi is used to host the website, manage the game, and bridge control from user to robots.
  -oled_pi: Contains a service script that runs on raspberry pi startup. This displays IP information of the raspberry pi. 

## Users/Roles

#### Guest

- can watch the game

#### Registered User

- can watch the game
- join/leave/confirm queue
- can play the game (wasd inputs)

#### Admin 

- The Admin shall be able to change match settings
- The Admin shhall be able shutdown the Robot

## Functional Requirements

### Website

- The website shall display a game livestream in under 10 second latency
- The website shall display a timer/scoreboard for the current match, showing the usernames of the players as well as their respective scores
- The website shall display the game queue
- The website shall display additional information such as About Us, How to Play, and Help
- The user shall be able to login and logout through Auth0
- The user shall be able to select a new username
- Newly registered users shall be able choose a unique username (2-15 characters, alphanumeric except for dashes ```-``` and underscores ```_```)

### Queue System

- Logged in users shall be able to click on a “Join Queue” button to enter the queue
- Users in the queue shall be able to click on a “Leave Queue” button to leave the queue
- The queue shall not allow duplicate users in the queue
- The queue shall display multiple upcoming matches by placing later matches below earlier matches
- When a match is done and there are at least two players in the queue, the system shall send a confirmation popup to the next two players in the queue with the option to “Accept” or “Decline”
  - In a confirmation popup, the player shall be able to click “Accept” within 10 seconds to agree to join the match
  - When both players of a confirmation popup click “Accept”, both players shall be able to enter a soccer robot match and each control a robot
  - In a confirmation popup, the player shall be able to click “Decline” within 10 seconds to refuse to join the match and leave the queue
  - In a confirmation popup, the system shall kick a player from the queue if the player does not click either “Accept” or “Decline” within 10 seconds

### Admin Panel

- User with an admin privileges can shutdown the robot.
- User with admin privileges can also change the match settings such as match length, num of players in a match.
- The Admin panel will be pn the secondary navbar adn when clicked we will get a popuo
  - The popup has two buttons that says shutdown and match settings
    - When users clicks on shutdown it verifies the user previleges and if admin it sends signal to the raspberry pi.
    - when clicked on match settings we wll get a drop down that asks for number of players and match length. The data is saved in the database when clicked save.

### Leaderboard

- The leaderboard shall show the columns of the players wins, losses, total goals scored, total games played, and win/loss ratio
- The leaderbooard should appear in modal
- The leaderboard shall be shortable by the columns representing: goals, wins, losses, win/loss ratio, and games played of each player
- The leaderboard shall show the top 5 players
- The leaderboard data shall update after every game

### Soccer Robot Game and Sending Signals

- The system shall be able to check whether the robots are in a "ready" state (as communicated by the Engineering team)
- A player who is entered into the soccer robot match shall be authorized and able to control one of the soccer robots for the duration of the match using WASD keys
- The system shall be able to receive “goal score” signals and update the current scoreboard accordingly
- Upon game completion, control of the soccer robots shall be revoked from the completed game’s players
- Upon game completion, the system shall reset the scoreboard scores to 0 and timer to its initial value

### Data Storage

- The system shall store, for each user, a unique username and email address
- The system shall store, for each completed match, the two players who played in the match, the final score of each player, and the datetime of the match

## Third Party Integrations

- Auth0 for authentication
- Twitch for live-streaming soccer arena
- Janus WebRTC Plugin for improved live streaming

## Tech Stack

- Nuxt.js
- Node.js
- SQLite and Prisma
- Postman

## Deployment Notes

N/A

## Migration Scripts

N/A

## Setting Up Development Environment

Ensure Node.js, npm, Visual Studio Code, Git, WSL2 (windows only) are installed. Auth0 account is needed.

### Install the repo

```bash
git clone https://github.com/UTDallasEPICS/Soccer-Robot-Web.git
cd Soccer-Robot-Web
npm install
```

### Set up Auth0

In the Auth0 application dashboard, configure the following with the appropriate port. Nuxt default port is 3000. The slashes ```/``` are very important. If using a different host than **localhost**, change **localhost** value accordingly.

- Allowed Callback URLs

```bash
http://localhost:PORT/api/callback
```

- Allowed Logout URLs

```bash
http://localhost:PORT/api/logoutcallback
```

- Allowed Web Origins

```bash
http://localhost:PORT/
```

Get the signing certificate from the Auth0 application and create a root-level cert-dev.pem containing the certificate.
```bash
-----BEGIN CERTIFICATE-----
...
-----END CERTIFICATE-----
```

### Initialize .env variables

#### Option #1

- Copy your mentor's .env file into the root directory

#### Option #2

- Create an .env file in the root directory
- Fill with necessary information based on **.env.example**

#### Some notes

- Some are ports duplicated because I wanted to follow naming convention without NUXT_ prefix. For example, all instances of **PORT2** should be the same port
- **LOCALHOST** can be localhost (for development) or host server's IP address
- **NUXT_AUTH0_CLIENTID**, **NUXT_AUTH0_SECRET**, and **NUXT_ISSUER** depends on Auth0 application
  - The slashes ```/``` are very important.
- **NUXT_CHANNEL_NAME** and **NUXT_PARENT_NAME** depends on Twitch account
- **CONTROLLER_PASSWORD** and **CONTROLLER_ACCESS** are useless because of randomized password rotations :)

### Set up database

```bash
npx prisma migrate dev --name init
```

### Start all servers

**MAKE SURE THE RASPBERRY SERVER IS RUNNING FIRST**.\
Then, in separate terminals, execute the following commands

```bash
# this starts Controller server
npm run bootcontrol
```

```bash
# this starts Game Manager server
npm run bootgame
```

```bash
# this starts Nuxt server
npm run dev
```

### Useful dev tips

#### Prisma GUI to view database

```bash
npx prisma studio
```

#### If exposing host IP address is needed, append ```-- --host```

```bash
npm run bootcontrol -- --host
npm run bootgame -- --host
npm run dev -- --host
```

### For development without Soccer Robot Hardware's Rasberry Pi ###
```bash
npm run bootpi -- --host
```

#### Custom package.json scripts

tsx is used to run Typescript files

```json
"scripts": {
    ...
    "bootgame": "tsx server/Game_Manager/src/index.ts",
    "bootcontrol": "tsx server/Controller/src/index.ts",
    ...
}
```

#### Use Postman to test HTTP and WebSockets

#### Mermaid is useful to document flowcharts and sequence diagrams in markdown

- <https://mermaid.js.org/>
- See ./notes

#### Documentation

See ./notes, ./figma
