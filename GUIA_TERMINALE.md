# Guida rapida: ROS 2, C++ e Git da terminale

Questa guida descrive il setup verificato sul Dell e i comandi quotidiani per lavorare dal terminale. Il progetto di esempio è `ros2_clion_smoke`.

## Ambiente configurato

- Ubuntu 26.04.1 LTS, architettura x86_64.
- ROS 2 Lyrical in `/opt/ros/lyrical`; Gazebo Jetty è già installato.
- CLion installato tramite JetBrains Toolbox e configurato per il progetto CMake/ROS 2.
- Git installato; identità commit: `Matteo <matteo.mascellaro@gmail.com>`; branch predefinito `main`.
- Workspace: `~/Projects/ros2_ws`; sorgenti ROS 2 in `~/Projects/ros2_ws/src`.
- Esempio C++: `~/Projects/ros2_ws/src/ros2_clion_smoke`.
- Repository remoto: `https://github.com/mattynadal10-sudo/ros2_clion_smoke.git` (privato).
- Il repository tiene sorgenti e configurazione condivisibile. Le cartelle generate `build/`, `install/`, `log/`, i file locali CLion e `CMakeUserPresets.json` sono escluse da Git.

Il repository è collegato al remoto e il commit di base è `39c787e`. L’accesso GitHub dell’IDE è separato dall’accesso Git nel terminale; vedere “Autenticazione GitHub da terminale”.

## Aprire un terminale ROS 2

Le nuove shell interattive leggono ROS 2 dal `~/.bashrc`. Per una shell o uno script che non carica `.bashrc`, attiva esplicitamente l’ambiente:

```bash
source /opt/ros/lyrical/setup.bash
```

Dopo aver compilato il workspace, attiva anche l’overlay locale (prima ROS, poi il workspace):

```bash
cd ~/Projects/ros2_ws
source /opt/ros/lyrical/setup.bash
source install/setup.bash
```

Per controllare che i comandi siano disponibili:

```bash
ros2 --help
colcon --help
git --version
```

## Struttura del workspace

```text
~/Projects/ros2_ws/
├── src/                 # pacchetti e repository Git
│   └── ros2_clion_smoke/ # singolo repository del package di esempio
├── build/               # generata da colcon; non versionare
├── install/             # generata da colcon; non versionare
└── log/                 # generata da colcon; non versionare
```

I package vivono dentro `src/`; `build/`, `install/` e `log/` sono al livello del workspace. Ogni package può essere un repository Git separato. Non inizializzare Git nella root del workspace se si vuole tenere un repository per package.

## Compilare ed eseguire il package di esempio

```bash
cd ~/Projects/ros2_ws
source /opt/ros/lyrical/setup.bash
colcon build --packages-select ros2_clion_smoke
source install/setup.bash
ros2 run ros2_clion_smoke hello_node
```

Per compilare tutti i package nel workspace, ometti `--packages-select ros2_clion_smoke`. Dopo aver aggiunto un package, `colcon build` dalla root lo scopre automaticamente. `--symlink-install` è facoltativo e utile durante lo sviluppo:

```bash
colcon build --symlink-install
```

Per creare un nuovo package C++ minimo con dipendenza `rclcpp`:

```bash
cd ~/Projects/ros2_ws/src
source /opt/ros/lyrical/setup.bash
ros2 pkg create --build-type ament_cmake --dependencies rclcpp nome_package
```

Poi aggiungi eseguibili e install rules al `CMakeLists.txt`, compila da `~/Projects/ros2_ws` con `colcon build`, e aggiungi il nuovo repository a Git se desideri sincronizzarlo.

## Usare CLion

Apri in CLion la cartella del package, ad esempio `~/Projects/ros2_ws/src/ros2_clion_smoke`. La configurazione condivisa è in `CMakePresets.json`; le impostazioni macchina-specifiche ROS/CLion sono locali in `CMakeUserPresets.json` e non vanno committate. Se CLion non trova ROS 2, controlla il profilo CMake locale e che il preset usi `/opt/ros/lyrical`.

## Git: ciclo normale

I comandi seguenti si eseguono nella directory del repository:

```bash
cd ~/Projects/ros2_ws/src/ros2_clion_smoke
git status
git pull --rebase
# modifica i file e, quando necessario, compila dalla root del workspace
git add README.md src/ CMakeLists.txt package.xml
git commit -m "Descrizione breve della modifica"
git push
```

Per un nuovo file specifico, aggiungi il suo percorso a `git add`. Prima di fare commit, controlla `git status`: non devono comparire output di build o file locali IDE. Il `.gitignore` del package esclude gli output e lo stato locale più comune.

Per vedere il branch e il remoto:

```bash
git branch -vv
git remote -v
git log --oneline --decorate -5
```

Se lavori anche sull’altro computer, fai `git pull --rebase` prima di modificare e `git push` dopo il commit. Evita di modificare gli stessi file contemporaneamente su due dispositivi.

## Clone su un altro computer Ubuntu

Installa/configura lì ROS 2, colcon, Git e CLion secondo il sistema di quel computer. Poi autentica GitHub (se il repository è privato) e clona:

```bash
mkdir -p ~/Projects/ros2_ws/src
cd ~/Projects/ros2_ws/src
git clone https://github.com/mattynadal10-sudo/ros2_clion_smoke.git
cd ~/Projects/ros2_ws
source /opt/ros/lyrical/setup.bash
colcon build
source install/setup.bash
```

Apri quindi `~/Projects/ros2_ws/src/ros2_clion_smoke` in CLion e imposta su quel PC il profilo CMake ROS locale, se non viene rilevato automaticamente. Le cartelle generate non si clonano: vengono ricreate con `colcon build`.

## Autenticazione GitHub da terminale

Il login GitHub dentro CLion non configura automaticamente Git nel terminale. Per HTTPS, il metodo consigliato è GitHub CLI (`gh`): installalo seguendo le istruzioni ufficiali per Ubuntu su [cli.github.com](https://cli.github.com/), quindi esegui:

```bash
gh auth login --hostname github.com --git-protocol https --web
gh auth setup-git
gh auth status
```

Completa il login nel browser quando `gh` mostra il codice di verifica. Poi `git pull` e `git push` useranno l’autenticazione configurata. Non inserire password o token direttamente nei comandi, perché possono finire nella cronologia della shell. In alternativa puoi configurare una chiave SSH personale su GitHub e usare un remoto `git@github.com:...`.

## Se compare un conflitto

Con `git pull --rebase`, Git ferma il rebase e indica i file in conflitto. Aprili in CLion, scegli e integra le modifiche corrette, poi:

```bash
git status
git add percorso/del/file-risolto
git rebase --continue
```

Ripeti per ogni conflitto. Se vuoi annullare il rebase e tornare allo stato precedente:

```bash
git rebase --abort
```

Dopo il rebase completato:

```bash
git push
```

Non usare `git push --force` per risolvere un conflitto. Se non sei sicuro di quale versione conservare, fermati e confronta le modifiche con `git diff` e `git diff --merge` prima di continuare.
