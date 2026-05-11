# Guide de mise en œuvre — LilyGO SMS Gateway

## Matériel nécessaire

| Élément | Détail |
|---|---|
| Module | LilyGO T-SIM7000G |
| Câble | USB-C (données, pas seulement charge) |
| Carte SIM | Orange.be — numéro +32456535814 |
| PC développeur | Windows 10/11 avec VS Code + PlatformIO |
| Réseau | WiFi `24_iot` accessible au module |
| MQTT Broker | Mosquitto sur `homeassitant.maison.net:1883` |

---

## Partie 1 — Premier flash USB

### 1.1 Ouvrir le projet

```powershell
cd C:\Users\dviel\lilygo-sms-gateway\firmware
```

Ou ouvrir `lilygo-sms-gateway.code-workspace` dans VS Code.

### 1.2 Vérifier le fichier `.env`

Le fichier `firmware/.env` est déjà configuré avec tes credentials. Vérifier :

```ini
WIFI_SSID=24_iot
MQTT_HOST=homeassitant.maison.net
SIM_APN=orange.be
SIM_PIN=1111
DEFAULT_RECIPIENT=+32476417968
OTA_PASSWORD=briga&K9
```

### 1.3 Brancher le module

1. Insérer la carte SIM dans le slot SIM du LilyGO (module éteint)
2. Brancher le câble USB-C sur le module → PC
3. Windows doit détecter un port COM (vérifier dans Gestionnaire de périphériques)

> Si aucun port COM n'apparaît : installer le driver CP210x ou CH340 selon la version du module.

### 1.4 Flash via USB

Dans le terminal VS Code (PowerShell) :

```powershell
pio run --environment lilygo-t-sim7000g --target upload
```

PlatformIO compile le firmware et le flashe automatiquement.

### 1.5 Ouvrir le moniteur série

```powershell
pio device monitor
```

Sortie attendue au démarrage (environ 30–60 s) :

```
[WiFi] Connecting......
[WiFi] IP: 192.168.x.x          ← noter cette IP pour l'OTA
[OTA] Ready — hostname: sms-gateway  port: 3232
[MQTT] Connecting... OK
[Modem] Waiting for network......... OK
[Modem] Signal: -75 dBm
```

> **Important** : noter l'adresse IP affichée — elle sera nécessaire pour les mises à jour OTA.

---

## Partie 2 — Test avec un client MQTT (sans Home Assistant)

Cette étape valide le fonctionnement complet avant d'impliquer HA.

### 2.1 Installer MQTT Explorer

Télécharger **MQTT Explorer** : [mqtt-explorer.com](http://mqtt-explorer.com)  
(client graphique Windows, gratuit)

### 2.2 Se connecter au broker

| Champ | Valeur |
|---|---|
| Host | `homeassitant.maison.net` |
| Port | `1883` |
| Username | *(vide)* |
| Password | *(vide)* |

Cliquer **Connect**.

### 2.3 Observer les topics actifs

Dans MQTT Explorer, les topics suivants doivent apparaître dès que le module est démarré :

| Topic | Contenu attendu |
|---|---|
| `sms/status` | `{"state":"online","rssi":-75,"uptime_s":120}` |

Le topic `sms/status` se rafraîchit toutes les 60 secondes.

### 2.4 Test — Envoyer un SMS depuis MQTT

Dans MQTT Explorer, **Publish** :

```
Topic   : sms/send
Payload : {"message": "Test depuis MQTT Explorer"}
```

→ Le SMS doit arriver sur ton téléphone (+32476417968) dans les 5–10 secondes.

Pour envoyer vers un autre numéro, ajouter le champ `recipient` :

```json
{"recipient": "+32499000000", "message": "Test numéro alternatif"}
```

### 2.5 Test — Recevoir un SMS depuis ton téléphone

Envoyer un SMS depuis ton téléphone (+32476417968) **vers la SIM gateway** (+32456535814).

Dans MQTT Explorer, le topic `sms/incoming` doit apparaître avec :

```json
{
  "sender": "+32476417968",
  "message": "Ton texte",
  "timestamp": "26/05/11,10:30:00+08"
}
```

### 2.6 Test — Vérifier la disponibilité (LWT)

Débrancher le module. Après quelques secondes, le broker publie automatiquement :

```
Topic   : sms/status
Payload : {"state":"offline"}
```

(Last Will Testament configuré dans le firmware)

---

## Partie 3 — Mises à jour OTA (sans câble USB)

Une fois le premier flash effectué, toutes les mises à jour suivantes se font via WiFi.

### 3.1 Mettre à jour l'IP dans `platformio.ini`

Ouvrir `firmware/platformio.ini` et remplacer l'IP placeholder :

```ini
[env:lilygo-t-sim7000g-ota]
...
upload_port = 192.168.x.x   ; ← remplacer par l'IP vue en partie 1.5
```

### 3.2 Définir le mot de passe OTA dans le terminal

```powershell
$env:OTA_PASSWORD = "briga&K9"
```

### 3.3 Lancer le flash OTA

```powershell
pio run --environment lilygo-t-sim7000g-ota --target upload
```

Progression dans le moniteur série du module :

```
[OTA] Start — type: firmware
[OTA] 10%
[OTA] 20%
...
[OTA] 100%
[OTA] Done — rebooting
```

> Le module redémarre seul après la mise à jour.

---

## Partie 4 — Intégration Home Assistant

### 4.1 Prérequis HA

- Intégration **MQTT** configurée dans HA : Paramètres → Intégrations → MQTT  
  Pointer sur `homeassitant.maison.net` port `1883`
- Accès SSH ou Samba à ton répertoire de config HA

### 4.2 Copier les fichiers YAML

Copier depuis le projet vers ton répertoire de config HA :

```
homeassistant/packages/sms_gateway.yaml     → config/packages/sms_gateway.yaml
homeassistant/automations/sms_alerts.yaml   → config/automations/sms_alerts.yaml
homeassistant/automations/sms_commands.yaml → config/automations/sms_commands.yaml
```

### 4.3 Activer les packages dans `configuration.yaml`

```yaml
homeassistant:
  packages:
    sms_gateway: !include packages/sms_gateway.yaml
```

### 4.4 Inclure les automations dans `automations.yaml`

```yaml
- !include automations/sms_alerts.yaml
- !include automations/sms_commands.yaml
```

### 4.5 Recharger la configuration

```
Outils développeur → YAML → Tout recharger
```

Ou redémarrer HA.

### 4.6 Vérifier les entités créées

Dans HA → Paramètres → Entités, rechercher `sms` :

| Entité | État attendu |
|---|---|
| `sensor.sms_gateway_etat` | `online` |
| `sensor.sms_gateway_rssi` | valeur en dBm (ex: `-75`) |
| `sensor.sms_gateway_uptime` | valeur en secondes |
| `sensor.dernier_sms_expediteur` | *(vide jusqu'au premier SMS reçu)* |
| `sensor.dernier_sms_message` | *(vide jusqu'au premier SMS reçu)* |

### 4.7 Adapter les entity_id des automations

Les automations `sms_alerts.yaml` utilisent des entity_id placeholder. Les remplacer par tes vraies entités :

| Placeholder | Remplacer par |
|---|---|
| `alarm_control_panel.home` | ton panneau d'alarme réel |
| `binary_sensor.motion_entree` | ton capteur de mouvement |
| `binary_sensor.porte_entree` | ton capteur de porte |
| `binary_sensor.fumee` | ton détecteur de fumée |
| `binary_sensor.ups_on_battery` | ton capteur de coupure secteur |

> Les alertes non pertinentes (sans le matériel correspondant) peuvent être désactivées dans HA
> sans supprimer le fichier.

### 4.8 Test final depuis HA

Dans **Outils développeur → Actions** :

```yaml
action: script.send_sms
data:
  message: "Test depuis Home Assistant"
```

→ SMS reçu sur +32476417968 ✓

Envoyer `STATUS` par SMS depuis ton téléphone vers la SIM gateway → réponse automatique ✓

---

## Résumé des topics MQTT

| Topic | Direction | Payload |
|---|---|---|
| `sms/status` | gateway → broker | `{"state":"online","rssi":-75,"uptime_s":3600}` |
| `sms/incoming` | gateway → broker | `{"sender":"+32476417968","message":"...","timestamp":"..."}` |
| `sms/send` | broker → gateway | `{"message":"..."}` ou `{"recipient":"+32...","message":"..."}` |

> `recipient` est optionnel dans `sms/send` — si absent, le SMS part vers +32476417968.

---

## Dépannage rapide

| Symptôme | Vérification |
|---|---|
| Pas de port COM détecté | Installer driver CH340 ou CP210x |
| `[WiFi] Connecting...` en boucle | Vérifier SSID/password dans `.env`, portée WiFi |
| `[MQTT] FAIL rc=-2` | Broker injoignable — vérifier IP et que Mosquitto tourne |
| `[Modem] Waiting for network...` en boucle | SIM mal insérée, PIN incorrect, réseau Orange absent |
| SMS envoyé mais non reçu | Vérifier solde/forfait SMS de la carte SIM |
| `sms/incoming` vide après SMS reçu | Vérifier AT+CNMI dans les logs série (redémarrer le module) |
| OTA `No response from device` | IP incorrecte dans `platformio.ini` ou module non joignable |
