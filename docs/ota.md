# Mise à jour OTA (Over-The-Air)

## Vue d'ensemble

La passerelle LilyGO T-SIM7000G supporte les mises à jour de firmware **sans câble USB**, via le réseau WiFi, grâce à la bibliothèque `ArduinoOTA` intégrée à l'ESP32 Arduino Core.

```
┌──────────────────┐   WiFi (même réseau)   ┌───────────────────┐
│  PC développeur  │ ─────────────────────→ │ LilyGO T-SIM7000G │
│  (PlatformIO)    │   port 3232 / espota   │  (OTA listener)   │
└──────────────────┘                        └───────────────────┘
```

> **Pré-requis** : le premier flash doit obligatoirement se faire par câble USB.  
> Les mises à jour suivantes peuvent être faites en OTA.

---

## 1. Configuration

### 1.1 Fichier `.env`

```ini
OTA_PASSWORD=mon_mot_de_passe_secret
OTA_HOSTNAME=lilygo-sms-gateway
```

> Ne pas laisser `OTA_PASSWORD` vide en production — n'importe qui sur le réseau pourrait flasher le device.

### 1.2 `platformio.ini` — environnement OTA

```ini
[env:lilygo-t-sim7000g-ota]
extends         = env:lilygo-t-sim7000g
upload_protocol = espota
upload_port     = 192.168.1.100   ; IP du device (voir §2)
upload_flags    =
    --auth=${sysenv.OTA_PASSWORD}
```

Remplacer `192.168.1.100` par l'adresse IP réelle (voir section 2).

---

## 2. Trouver l'adresse IP du device

Au démarrage, la passerelle affiche son IP dans le moniteur série :

```
[WiFi] IP: 192.168.1.100
[OTA] Ready — hostname: lilygo-sms-gateway  port: 3232
```

**Méthodes alternatives :**

| Méthode | Commande / Action |
|---|---|
| Moniteur série PlatformIO | `pio device monitor` |
| Interface DHCP du routeur | Chercher `lilygo-sms-gateway` |
| mDNS (Windows avec Bonjour) | `ping lilygo-sms-gateway.local` |

---

## 3. Premier flash — câble USB

```bash
cd firmware
pio run --target upload --environment lilygo-t-sim7000g
pio device monitor
```

Vérifier dans le moniteur que la ligne OTA apparaît :

```
[OTA] Ready — hostname: lilygo-sms-gateway  port: 3232
```

---

## 4. Flash OTA — procédure normale

### 4.1 Mettre à jour l'IP dans `platformio.ini`

```ini
upload_port = 192.168.1.100   ; IP trouvée à l'étape 2
```

### 4.2 Définir le mot de passe dans l'environnement

```bash
# Linux / macOS / Git Bash (Windows)
export OTA_PASSWORD=mon_mot_de_passe_secret

# PowerShell (Windows)
$env:OTA_PASSWORD = "mon_mot_de_passe_secret"
```

### 4.3 Lancer le flash OTA

```bash
pio run --target upload --environment lilygo-t-sim7000g-ota
```

Sortie attendue dans PlatformIO :

```
Uploading .pio/build/lilygo-t-sim7000g-ota/firmware.bin
...
[100%] Upload done.
```

Et dans le moniteur série du device :

```
[OTA] 0%
[OTA] 10%
...
[OTA] 100%
[OTA] Done — rebooting
```

---

## 5. Sécurité

| Risque | Mitigation |
|---|---|
| Flash non autorisé | Toujours définir `OTA_PASSWORD` |
| Interception réseau | Utiliser un réseau WiFi privé (WPA2/WPA3) |
| Brick sur coupure de courant | L'ESP32 intègre un mécanisme de rollback automatique |
| Accès réseau non désiré | Isoler le device sur un VLAN IoT si possible |

---

## 6. Dépannage

| Symptôme | Cause probable | Solution |
|---|---|---|
| `No response from device` | IP incorrecte ou device non joignable | Vérifier IP, pinger le device |
| `Authentication Failed` | Mauvais mot de passe | Vérifier `OTA_PASSWORD` dans l'env shell |
| `Connection refused` | OTA non démarré (WiFi non connecté) | Vérifier logs WiFi au démarrage |
| Timeout à 0% | Firewall bloque port 3232 | Autoriser UDP 3232 et TCP 3232 sur le PC |
| Device redémarre en boucle | Firmware corrompu | Reflasher via USB |

---

## 7. Intégration CI/CD (optionnel)

Pour automatiser le flash OTA dans un pipeline GitHub Actions ou similaire :

```yaml
- name: Flash OTA
  env:
    OTA_PASSWORD: ${{ secrets.OTA_PASSWORD }}
  run: |
    cd firmware
    pio run --target upload --environment lilygo-t-sim7000g-ota
```

Stocker `OTA_PASSWORD` dans les secrets du dépôt, jamais en clair.
