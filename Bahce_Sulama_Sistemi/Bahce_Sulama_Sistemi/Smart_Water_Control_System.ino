#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <Preferences.h>
#include <virtuabotixRTC.h>
#include <anasayfa.h>
#include <kontrolpaneli.h>
#include <kontrolPaneliDoldurma.h>

#define CLK_PIN 26 //D5. pini clock pini olarak tanımladık
#define DAT_PIN 25 //D6 pini data pini olarak tanımladık
#define RST_PIN 33 //D7 pini reset pini olarak tanımladık.

virtuabotixRTC myRTC(CLK_PIN, DAT_PIN, RST_PIN);

WebServer server(80);

Preferences preferences;

///// String ayırma fonksiyonu //////////////////////////////////////

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++)
  {
    if (data.charAt(i) == separator || i == maxIndex)
    {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

////////////////////////////////////////////////////////////////////////////

String daysOfWeek[] = {"pazar", "pazartesi", "sali", "carsamba", "persembe", "cuma", "cumartesi"};

unsigned long int sure = 10000;
unsigned long int t = 0;
unsigned long int t2 = 0;

struct vakit
{
  String name;
  String bitisName;
  int baslangicSaat = 00;
  int baslangicDakika = 00;
  int bitisSaat = 00;
  int bitisDakika = 00;

  void vakitCek()
  {
    String vakit = server.arg(name);
    if (getValue(vakit, ':', 0).toInt() == 0 && getValue(vakit, ':', 1).toInt() == 0)
    {
      Serial.println("Başlangıç: Değişmedi");
    }
    else
    {
      baslangicSaat = getValue(vakit, ':', 0).toInt();
      baslangicDakika = getValue(vakit, ':', 1).toInt();

      Serial.println("Baslangic: " + vakit);
    }

    vakit = server.arg(bitisName);
    if (getValue(vakit, ':', 0).toInt() == 0 && getValue(vakit, ':', 1).toInt() == 0)
    {
      Serial.println("Bitiş: Değişmedi");
    }
    else
    {
      bitisSaat = getValue(vakit, ':', 0).toInt();
      bitisDakika = getValue(vakit, ':', 1).toInt();

      Serial.println("Bitis: " + vakit);
    }
  }

  bool kontrol()
  {
    if (myRTC.hours > baslangicSaat)
    {
      if (bitisSaat > myRTC.hours)
      {
        return true;
      }
      else if (bitisSaat == myRTC.hours)
      {
        if (myRTC.minutes < bitisDakika)
        {
          return true;
        }
        else
        {
          return false;
        }
      }
      else
      {
        return false;
      }
    }
    else if (myRTC.hours == baslangicSaat)
    {
      if (myRTC.minutes >= baslangicDakika)
      {
        if (bitisSaat > myRTC.hours)
        {
          return true;
        }
        else if (bitisSaat == myRTC.hours)
        {
          if (myRTC.minutes < bitisDakika)
          {
            return true;
          }
          else
          {
            return false;
          }
        }
        else
        {
          return false;
        }
      }
      else
      {
        return false;
      }
    }
    else
    {
      return false;
    }
  }

  void sifirla()
  {
    baslangicSaat = 0;
    baslangicDakika = 0;
    bitisDakika = 0;
    bitisSaat = 0;
  }
};

struct gun
{
  vakit vakit_1, vakit_2, vakit_3;

  String name;

  void vakitAdlariTanimla()
  {
    vakit_1.name = "saat1";
    vakit_1.bitisName = "saat1b";
    vakit_2.name = "saat2";
    vakit_2.bitisName = "saat2b";
    vakit_3.name = "saat3";
    vakit_3.bitisName = "saat3b";
  }

  void saatleriCekme()
  {
    vakit_1.vakitCek();
    vakit_2.vakitCek();
    vakit_3.vakitCek();
  }

  bool kontrol()
  {
    if (daysOfWeek[myRTC.dayofweek - 1] == name)
    {
      if (vakit_1.kontrol() == true || vakit_2.kontrol() == true || vakit_3.kontrol() == true)
      {
        return true;
      }
      else
      {
        return false;
      }
    }
    else
    {
      return false;
    }
  }

  void sifirla()
  {
    vakit_1.sifirla();
    vakit_2.sifirla();
    vakit_3.sifirla();
  }
};

struct role
{
  bool roleStatus = LOW;
  bool vakitGeldi = LOW;
  bool temp = LOW;
  bool degisti = false;
  String roleName;
  int pin;

  gun pazartesi, sali, carsamba, persembe, cuma, cumartesi, pazar;

  void tanimlamalar()
  {
    pazartesi.name = "pazartesi";
    sali.name = "sali";
    carsamba.name = "carsamba";
    persembe.name = "persembe";
    cuma.name = "cuma";
    cumartesi.name = "cumartesi";
    pazar.name = "pazar";
    pazartesi.vakitAdlariTanimla();
    sali.vakitAdlariTanimla();
    carsamba.vakitAdlariTanimla();
    persembe.vakitAdlariTanimla();
    cuma.vakitAdlariTanimla();
    cumartesi.vakitAdlariTanimla();
    pazar.vakitAdlariTanimla();
  }

  void formCekme()
  {
    Serial.println(server.arg("gün"));
    if (server.arg("gün") == "pazartesi")
    {
      pazartesi.saatleriCekme();
    }

    else if (server.arg("gün") == "sali")
    {
      sali.saatleriCekme();
    }

    else if (server.arg("gün") == "carsamba")
    {
      carsamba.saatleriCekme();
    }

    else if (server.arg("gün") == "persembe")
    {
      persembe.saatleriCekme();
    }

    else if (server.arg("gün") == "cuma")
    {
      cuma.saatleriCekme();
    }

    else if (server.arg("gün") == "cumartesi")
    {
      cumartesi.saatleriCekme();
    }

    else if (server.arg("gün") == "pazar")
    {
      pazar.saatleriCekme();
    }
  }

  void kontrolPaneliDoldur(String secilenGun)
  {
    if (secilenGun == "pazartesi")
    {
      server.send(200, "text/html", SendKontrolPaneli(roleName, secilenGun, roleStatus, String(pazartesi.vakit_1.baslangicSaat) + ":" + String(pazartesi.vakit_1.baslangicDakika), String(pazartesi.vakit_1.bitisSaat) + ":" + String(pazartesi.vakit_1.bitisDakika), String(pazartesi.vakit_2.baslangicSaat) + ":" + String(pazartesi.vakit_2.baslangicDakika), String(pazartesi.vakit_2.bitisSaat) + ":" + String(pazartesi.vakit_2.bitisDakika), String(pazartesi.vakit_3.baslangicSaat) + ":" + String(pazartesi.vakit_3.baslangicDakika), String(pazartesi.vakit_3.bitisSaat) + ":" + String(pazartesi.vakit_3.bitisDakika)));
    }

    else if (secilenGun == "sali")
    {
      server.send(200, "text/html", SendKontrolPaneli(roleName, secilenGun, roleStatus, String(sali.vakit_1.baslangicSaat) + ":" + String(sali.vakit_1.baslangicDakika), String(sali.vakit_1.bitisSaat) + ":" + String(sali.vakit_1.bitisDakika), String(sali.vakit_2.baslangicSaat) + ":" + String(sali.vakit_2.baslangicDakika), String(sali.vakit_2.bitisSaat) + ":" + String(sali.vakit_2.bitisDakika), String(sali.vakit_3.baslangicSaat) + ":" + String(sali.vakit_3.baslangicDakika), String(sali.vakit_3.bitisSaat) + ":" + String(sali.vakit_3.bitisDakika)));
    }

    else if (secilenGun == "carsamba")
    {
      server.send(200, "text/html", SendKontrolPaneli(roleName, secilenGun, roleStatus, String(carsamba.vakit_1.baslangicSaat) + ":" + String(carsamba.vakit_1.baslangicDakika), String(carsamba.vakit_1.bitisSaat) + ":" + String(carsamba.vakit_1.bitisDakika), String(carsamba.vakit_2.baslangicSaat) + ":" + String(carsamba.vakit_2.baslangicDakika), String(carsamba.vakit_2.bitisSaat) + ":" + String(carsamba.vakit_2.bitisDakika), String(carsamba.vakit_3.baslangicSaat) + ":" + String(carsamba.vakit_3.baslangicDakika), String(carsamba.vakit_3.bitisSaat) + ":" + String(carsamba.vakit_3.bitisDakika)));
    }

    else if (secilenGun == "persembe")
    {
      server.send(200, "text/html", SendKontrolPaneli(roleName, secilenGun, roleStatus, String(persembe.vakit_1.baslangicSaat) + ":" + String(persembe.vakit_1.baslangicDakika), String(persembe.vakit_1.bitisSaat) + ":" + String(persembe.vakit_1.bitisDakika), String(persembe.vakit_2.baslangicSaat) + ":" + String(persembe.vakit_2.baslangicDakika), String(persembe.vakit_2.bitisSaat) + ":" + String(persembe.vakit_2.bitisDakika), String(persembe.vakit_3.baslangicSaat) + ":" + String(persembe.vakit_3.baslangicDakika), String(persembe.vakit_3.bitisSaat) + ":" + String(persembe.vakit_3.bitisDakika)));
    }

    else if (secilenGun == "cuma")
    {
      server.send(200, "text/html", SendKontrolPaneli(roleName, secilenGun, roleStatus, String(cuma.vakit_1.baslangicSaat) + ":" + String(cuma.vakit_1.baslangicDakika), String(cuma.vakit_1.bitisSaat) + ":" + String(cuma.vakit_1.bitisDakika), String(cuma.vakit_2.baslangicSaat) + ":" + String(cuma.vakit_2.baslangicDakika), String(cuma.vakit_2.bitisSaat) + ":" + String(cuma.vakit_2.bitisDakika), String(cuma.vakit_3.baslangicSaat) + ":" + String(cuma.vakit_3.baslangicDakika), String(cuma.vakit_3.bitisSaat) + ":" + String(cuma.vakit_3.bitisDakika)));
    }

    else if (secilenGun == "cumartesi")
    {
      server.send(200, "text/html", SendKontrolPaneli(roleName, secilenGun, roleStatus, String(cumartesi.vakit_1.baslangicSaat) + ":" + String(cumartesi.vakit_1.baslangicDakika), String(cumartesi.vakit_1.bitisSaat) + ":" + String(cumartesi.vakit_1.bitisDakika), String(cumartesi.vakit_2.baslangicSaat) + ":" + String(cumartesi.vakit_2.baslangicDakika), String(cumartesi.vakit_2.bitisSaat) + ":" + String(cumartesi.vakit_2.bitisDakika), String(cumartesi.vakit_3.baslangicSaat) + ":" + String(cumartesi.vakit_3.baslangicDakika), String(cumartesi.vakit_3.bitisSaat) + ":" + String(cumartesi.vakit_3.bitisDakika)));
    }

    else if (secilenGun == "pazar")
    {
      server.send(200, "text/html", SendKontrolPaneli(roleName, secilenGun, roleStatus, String(pazar.vakit_1.baslangicSaat) + ":" + String(pazar.vakit_1.baslangicDakika), String(pazar.vakit_1.bitisSaat) + ":" + String(pazar.vakit_1.bitisDakika), String(pazar.vakit_2.baslangicSaat) + ":" + String(pazar.vakit_2.baslangicDakika), String(pazar.vakit_2.bitisSaat) + ":" + String(pazar.vakit_2.bitisDakika), String(pazar.vakit_3.baslangicSaat) + ":" + String(pazar.vakit_3.baslangicDakika), String(pazar.vakit_3.bitisSaat) + ":" + String(pazar.vakit_3.bitisDakika)));
    }
  }

  void kontrol()
  {
    if ((pazartesi.kontrol() == true) || (sali.kontrol() == true) || (carsamba.kontrol() == true) ||
        (persembe.kontrol() == true) || (cuma.kontrol() == true) || (cumartesi.kontrol() == true) || (pazar.kontrol() == true))
    {
      vakitGeldi = HIGH;
      if (degisti == true)
      {
        roleStatus = HIGH;
        Serial.println(roleName + ": Vakit Geldi");
        degisti = false;
      }
    }

    else
    {
      vakitGeldi = LOW;
      if (degisti == true)
      {
        roleStatus = LOW;
        Serial.println(roleName + ": Vakit Gelmedi");
        degisti = false;
      }
    }

    if (vakitGeldi != temp)
    {
      Serial.println(roleName+" :Vana Durumu Değişti");
      degisti = true;
      temp = vakitGeldi;
    }
  }

  void gunsifirla(String secilenGun)
  {
    if (secilenGun == "pazartesi")
    {
      pazartesi.sifirla();
    }
    else if (secilenGun == "sali")
    {
      sali.sifirla();
    }
    else if (secilenGun == "carsamba")
    {
      carsamba.sifirla();
    }
    else if (secilenGun == "persembe")
    {
      persembe.sifirla();
    }
    else if (secilenGun == "cuma")
    {
      cuma.sifirla();
    }
    else if (secilenGun == "cumartesi")
    {
      cumartesi.sifirla();
    }
    else if (secilenGun == "pazar")
    {
      pazar.sifirla();
    }
  }
};

role role_1, role_2, role_3, role_4;

void tanimlama()
{
  role_1.roleName = "1. Vana";
  role_2.roleName = "2. Vana";
  role_3.roleName = "3. Vana";
  role_4.roleName = "4. Vana";
  role_1.pin = 13;
  role_2.pin = 12;
  role_3.pin = 14;
  role_4.pin = 27;
  role_1.tanimlamalar();
  role_2.tanimlamalar();
  role_3.tanimlamalar();
  role_4.tanimlamalar();
}

void kaydetRole1Baslangic()
{
  preferences.putInt("r1_pt_v1_ba_s", role_1.pazartesi.vakit_1.baslangicSaat);
  preferences.putInt("r1_pt_v2_ba_s", role_1.pazartesi.vakit_2.baslangicSaat);
  preferences.putInt("r1_pt_v3_ba_s", role_1.pazartesi.vakit_3.baslangicSaat);

  preferences.putInt("r1_pt_v1_ba_d", role_1.pazartesi.vakit_1.baslangicDakika);
  preferences.putInt("r1_pt_v2_ba_d", role_1.pazartesi.vakit_2.baslangicDakika);
  preferences.putInt("r1_pt_v3_ba_d", role_1.pazartesi.vakit_3.baslangicDakika);

  preferences.putInt("r1_sl_v1_ba_s", role_1.sali.vakit_1.baslangicSaat);
  preferences.putInt("r1_sl_v2_ba_s", role_1.sali.vakit_2.baslangicSaat);
  preferences.putInt("r1_sl_v3_ba_s", role_1.sali.vakit_3.baslangicSaat);

  preferences.putInt("r1_sl_v1_ba_d", role_1.sali.vakit_1.baslangicDakika);
  preferences.putInt("r1_sl_v2_ba_d", role_1.sali.vakit_2.baslangicDakika);
  preferences.putInt("r1_sl_v3_ba_d", role_1.sali.vakit_3.baslangicDakika);

  preferences.putInt("r1_cr_v1_ba_s", role_1.carsamba.vakit_1.baslangicSaat);
  preferences.putInt("r1_cr_v2_ba_s", role_1.carsamba.vakit_2.baslangicSaat);
  preferences.putInt("r1_cr_v3_ba_s", role_1.carsamba.vakit_3.baslangicSaat);

  preferences.putInt("r1_cr_v1_ba_d", role_1.carsamba.vakit_1.baslangicDakika);
  preferences.putInt("r1_cr_v2_ba_d", role_1.carsamba.vakit_2.baslangicDakika);
  preferences.putInt("r1_cr_v3_ba_d", role_1.carsamba.vakit_3.baslangicDakika);

  preferences.putInt("r1_pr_v1_ba_s", role_1.persembe.vakit_1.baslangicSaat);
  preferences.putInt("r1_pr_v2_ba_s", role_1.persembe.vakit_2.baslangicSaat);
  preferences.putInt("r1_pr_v3_ba_s", role_1.persembe.vakit_3.baslangicSaat);

  preferences.putInt("r1_pr_v1_ba_d", role_1.persembe.vakit_1.baslangicDakika);
  preferences.putInt("r1_pr_v2_ba_d", role_1.persembe.vakit_2.baslangicDakika);
  preferences.putInt("r1_pr_v3_ba_d", role_1.persembe.vakit_3.baslangicDakika);

  preferences.putInt("r1_cu_v1_ba_s", role_1.cuma.vakit_1.baslangicSaat);
  preferences.putInt("r1_cu_v2_ba_s", role_1.cuma.vakit_2.baslangicSaat);
  preferences.putInt("r1_cu_v3_ba_s", role_1.cuma.vakit_3.baslangicSaat);

  preferences.putInt("r1_cu_v1_ba_d", role_1.cuma.vakit_1.baslangicDakika);
  preferences.putInt("r1_cu_v2_ba_d", role_1.cuma.vakit_2.baslangicDakika);
  preferences.putInt("r1_cu_v3_ba_d", role_1.cuma.vakit_3.baslangicDakika);

  preferences.putInt("r1_ct_v1_ba_s", role_1.cumartesi.vakit_1.baslangicSaat);
  preferences.putInt("r1_ct_v2_ba_s", role_1.cumartesi.vakit_2.baslangicSaat);
  preferences.putInt("r1_ct_v3_ba_s", role_1.cumartesi.vakit_3.baslangicSaat);

  preferences.putInt("r1_ct_v1_ba_d", role_1.cumartesi.vakit_1.baslangicDakika);
  preferences.putInt("r1_ct_v2_ba_d", role_1.cumartesi.vakit_2.baslangicDakika);
  preferences.putInt("r1_ct_v3_ba_d", role_1.cumartesi.vakit_3.baslangicDakika);

  preferences.putInt("r1_pz_v1_ba_s", role_1.pazar.vakit_1.baslangicSaat);
  preferences.putInt("r1_pz_v2_ba_s", role_1.pazar.vakit_2.baslangicSaat);
  preferences.putInt("r1_pz_v3_ba_s", role_1.pazar.vakit_3.baslangicSaat);

  preferences.putInt("r1_pz_v1_ba_d", role_1.pazar.vakit_1.baslangicDakika);
  preferences.putInt("r1_pz_v2_ba_d", role_1.pazar.vakit_2.baslangicDakika);
  preferences.putInt("r1_pz_v3_ba_d", role_1.pazar.vakit_3.baslangicDakika);
}

void kaydetRole2Baslangic()
{
  preferences.putInt("r2_pt_v1_ba_s", role_2.pazartesi.vakit_1.baslangicSaat);
  preferences.putInt("r2_pt_v2_ba_s", role_2.pazartesi.vakit_2.baslangicSaat);
  preferences.putInt("r2_pt_v3_ba_s", role_2.pazartesi.vakit_3.baslangicSaat);

  preferences.putInt("r2_pt_v1_ba_d", role_2.pazartesi.vakit_1.baslangicDakika);
  preferences.putInt("r2_pt_v2_ba_d", role_2.pazartesi.vakit_2.baslangicDakika);
  preferences.putInt("r2_pt_v3_ba_d", role_2.pazartesi.vakit_3.baslangicDakika);

  preferences.putInt("r2_sl_v1_ba_s", role_2.sali.vakit_1.baslangicSaat);
  preferences.putInt("r2_sl_v2_ba_s", role_2.sali.vakit_2.baslangicSaat);
  preferences.putInt("r2_sl_v3_ba_s", role_2.sali.vakit_3.baslangicSaat);

  preferences.putInt("r2_sl_v1_ba_d", role_2.sali.vakit_1.baslangicDakika);
  preferences.putInt("r2_sl_v2_ba_d", role_2.sali.vakit_2.baslangicDakika);
  preferences.putInt("r2_sl_v3_ba_d", role_2.sali.vakit_3.baslangicDakika);

  preferences.putInt("r2_cr_v1_ba_s", role_2.carsamba.vakit_1.baslangicSaat);
  preferences.putInt("r2_cr_v2_ba_s", role_2.carsamba.vakit_2.baslangicSaat);
  preferences.putInt("r2_cr_v3_ba_s", role_2.carsamba.vakit_3.baslangicSaat);

  preferences.putInt("r2_cr_v1_ba_d", role_2.carsamba.vakit_1.baslangicDakika);
  preferences.putInt("r2_cr_v2_ba_d", role_2.carsamba.vakit_2.baslangicDakika);
  preferences.putInt("r2_cr_v3_ba_d", role_2.carsamba.vakit_3.baslangicDakika);

  preferences.putInt("r2_pr_v1_ba_s", role_2.persembe.vakit_1.baslangicSaat);
  preferences.putInt("r2_pr_v2_ba_s", role_2.persembe.vakit_2.baslangicSaat);
  preferences.putInt("r2_pr_v3_ba_s", role_2.persembe.vakit_3.baslangicSaat);

  preferences.putInt("r2_pr_v1_ba_d", role_2.persembe.vakit_1.baslangicDakika);
  preferences.putInt("r2_pr_v2_ba_d", role_2.persembe.vakit_2.baslangicDakika);
  preferences.putInt("r2_pr_v3_ba_d", role_2.persembe.vakit_3.baslangicDakika);

  preferences.putInt("r2_cu_v1_ba_s", role_2.cuma.vakit_1.baslangicSaat);
  preferences.putInt("r2_cu_v2_ba_s", role_2.cuma.vakit_2.baslangicSaat);
  preferences.putInt("r2_cu_v3_ba_s", role_2.cuma.vakit_3.baslangicSaat);

  preferences.putInt("r2_cu_v1_ba_d", role_2.cuma.vakit_1.baslangicDakika);
  preferences.putInt("r2_cu_v2_ba_d", role_2.cuma.vakit_2.baslangicDakika);
  preferences.putInt("r2_cu_v3_ba_d", role_2.cuma.vakit_3.baslangicDakika);

  preferences.putInt("r2_ct_v1_ba_s", role_2.cumartesi.vakit_1.baslangicSaat);
  preferences.putInt("r2_ct_v2_ba_s", role_2.cumartesi.vakit_2.baslangicSaat);
  preferences.putInt("r2_ct_v3_ba_s", role_2.cumartesi.vakit_3.baslangicSaat);

  preferences.putInt("r2_ct_v1_ba_d", role_2.cumartesi.vakit_1.baslangicDakika);
  preferences.putInt("r2_ct_v2_ba_d", role_2.cumartesi.vakit_2.baslangicDakika);
  preferences.putInt("r2_ct_v3_ba_d", role_2.cumartesi.vakit_3.baslangicDakika);

  preferences.putInt("r2_pz_v1_ba_s", role_2.pazar.vakit_1.baslangicSaat);
  preferences.putInt("r2_pz_v2_ba_s", role_2.pazar.vakit_2.baslangicSaat);
  preferences.putInt("r2_pz_v3_ba_s", role_2.pazar.vakit_3.baslangicSaat);

  preferences.putInt("r2_pz_v1_ba_d", role_2.pazar.vakit_1.baslangicDakika);
  preferences.putInt("r2_pz_v2_ba_d", role_2.pazar.vakit_2.baslangicDakika);
  preferences.putInt("r2_pz_v3_ba_d", role_2.pazar.vakit_3.baslangicDakika);
}

void kaydetRole3Baslangic()
{
  preferences.putInt("r3_pt_v1_ba_s", role_3.pazartesi.vakit_1.baslangicSaat);
  preferences.putInt("r3_pt_v2_ba_s", role_3.pazartesi.vakit_2.baslangicSaat);
  preferences.putInt("r3_pt_v3_ba_s", role_3.pazartesi.vakit_3.baslangicSaat);

  preferences.putInt("r3_pt_v1_ba_d", role_3.pazartesi.vakit_1.baslangicDakika);
  preferences.putInt("r3_pt_v2_ba_d", role_3.pazartesi.vakit_2.baslangicDakika);
  preferences.putInt("r3_pt_v3_ba_d", role_3.pazartesi.vakit_3.baslangicDakika);

  preferences.putInt("r3_sl_v1_ba_s", role_3.sali.vakit_1.baslangicSaat);
  preferences.putInt("r3_sl_v2_ba_s", role_3.sali.vakit_2.baslangicSaat);
  preferences.putInt("r3_sl_v3_ba_s", role_3.sali.vakit_3.baslangicSaat);

  preferences.putInt("r3_sl_v1_ba_d", role_3.sali.vakit_1.baslangicDakika);
  preferences.putInt("r3_sl_v2_ba_d", role_3.sali.vakit_2.baslangicDakika);
  preferences.putInt("r3_sl_v3_ba_d", role_3.sali.vakit_3.baslangicDakika);

  preferences.putInt("r3_cr_v1_ba_s", role_3.carsamba.vakit_1.baslangicSaat);
  preferences.putInt("r3_cr_v2_ba_s", role_3.carsamba.vakit_2.baslangicSaat);
  preferences.putInt("r3_cr_v3_ba_s", role_3.carsamba.vakit_3.baslangicSaat);

  preferences.putInt("r3_cr_v1_ba_d", role_3.carsamba.vakit_1.baslangicDakika);
  preferences.putInt("r3_cr_v2_ba_d", role_3.carsamba.vakit_2.baslangicDakika);
  preferences.putInt("r3_cr_v3_ba_d", role_3.carsamba.vakit_3.baslangicDakika);

  preferences.putInt("r3_pr_v1_ba_s", role_3.persembe.vakit_1.baslangicSaat);
  preferences.putInt("r3_pr_v2_ba_s", role_3.persembe.vakit_2.baslangicSaat);
  preferences.putInt("r3_pr_v3_ba_s", role_3.persembe.vakit_3.baslangicSaat);

  preferences.putInt("r3_pr_v1_ba_d", role_3.persembe.vakit_1.baslangicDakika);
  preferences.putInt("r3_pr_v2_ba_d", role_3.persembe.vakit_2.baslangicDakika);
  preferences.putInt("r3_pr_v3_ba_d", role_3.persembe.vakit_3.baslangicDakika);

  preferences.putInt("r3_cu_v1_ba_s", role_3.cuma.vakit_1.baslangicSaat);
  preferences.putInt("r3_cu_v2_ba_s", role_3.cuma.vakit_2.baslangicSaat);
  preferences.putInt("r3_cu_v3_ba_s", role_3.cuma.vakit_3.baslangicSaat);

  preferences.putInt("r3_cu_v1_ba_d", role_3.cuma.vakit_1.baslangicDakika);
  preferences.putInt("r3_cu_v2_ba_d", role_3.cuma.vakit_2.baslangicDakika);
  preferences.putInt("r3_cu_v3_ba_d", role_3.cuma.vakit_3.baslangicDakika);

  preferences.putInt("r3_ct_v1_ba_s", role_3.cumartesi.vakit_1.baslangicSaat);
  preferences.putInt("r3_ct_v2_ba_s", role_3.cumartesi.vakit_2.baslangicSaat);
  preferences.putInt("r3_ct_v3_ba_s", role_3.cumartesi.vakit_3.baslangicSaat);

  preferences.putInt("r3_ct_v1_ba_d", role_3.cumartesi.vakit_1.baslangicDakika);
  preferences.putInt("r3_ct_v2_ba_d", role_3.cumartesi.vakit_2.baslangicDakika);
  preferences.putInt("r3_ct_v3_ba_d", role_3.cumartesi.vakit_3.baslangicDakika);

  preferences.putInt("r3_pz_v1_ba_s", role_3.pazar.vakit_1.baslangicSaat);
  preferences.putInt("r3_pz_v2_ba_s", role_3.pazar.vakit_2.baslangicSaat);
  preferences.putInt("r3_pz_v3_ba_s", role_3.pazar.vakit_3.baslangicSaat);

  preferences.putInt("r3_pz_v1_ba_d", role_3.pazar.vakit_1.baslangicDakika);
  preferences.putInt("r3_pz_v2_ba_d", role_3.pazar.vakit_2.baslangicDakika);
  preferences.putInt("r3_pz_v3_ba_d", role_3.pazar.vakit_3.baslangicDakika);
}

void kaydetRole4Baslangic()
{
  preferences.putInt("r4_pt_v1_ba_s", role_4.pazartesi.vakit_1.baslangicSaat);
  preferences.putInt("r4_pt_v2_ba_s", role_4.pazartesi.vakit_2.baslangicSaat);
  preferences.putInt("r4_pt_v3_ba_s", role_4.pazartesi.vakit_3.baslangicSaat);

  preferences.putInt("r4_pt_v1_ba_d", role_4.pazartesi.vakit_1.baslangicDakika);
  preferences.putInt("r4_pt_v2_ba_d", role_4.pazartesi.vakit_2.baslangicDakika);
  preferences.putInt("r4_pt_v3_ba_d", role_4.pazartesi.vakit_3.baslangicDakika);

  preferences.putInt("r4_sl_v1_ba_s", role_4.sali.vakit_1.baslangicSaat);
  preferences.putInt("r4_sl_v2_ba_s", role_4.sali.vakit_2.baslangicSaat);
  preferences.putInt("r4_sl_v3_ba_s", role_4.sali.vakit_3.baslangicSaat);

  preferences.putInt("r4_sl_v1_ba_d", role_4.sali.vakit_1.baslangicDakika);
  preferences.putInt("r4_sl_v2_ba_d", role_4.sali.vakit_2.baslangicDakika);
  preferences.putInt("r4_sl_v3_ba_d", role_4.sali.vakit_3.baslangicDakika);

  preferences.putInt("r4_cr_v1_ba_s", role_4.carsamba.vakit_1.baslangicSaat);
  preferences.putInt("r4_cr_v2_ba_s", role_4.carsamba.vakit_2.baslangicSaat);
  preferences.putInt("r4_cr_v3_ba_s", role_4.carsamba.vakit_3.baslangicSaat);

  preferences.putInt("r4_cr_v1_ba_d", role_4.carsamba.vakit_1.baslangicDakika);
  preferences.putInt("r4_cr_v2_ba_d", role_4.carsamba.vakit_2.baslangicDakika);
  preferences.putInt("r4_cr_v3_ba_d", role_4.carsamba.vakit_3.baslangicDakika);

  preferences.putInt("r4_pr_v1_ba_s", role_4.persembe.vakit_1.baslangicSaat);
  preferences.putInt("r4_pr_v2_ba_s", role_4.persembe.vakit_2.baslangicSaat);
  preferences.putInt("r4_pr_v3_ba_s", role_4.persembe.vakit_3.baslangicSaat);

  preferences.putInt("r4_pr_v1_ba_d", role_4.persembe.vakit_1.baslangicDakika);
  preferences.putInt("r4_pr_v2_ba_d", role_4.persembe.vakit_2.baslangicDakika);
  preferences.putInt("r4_pr_v3_ba_d", role_4.persembe.vakit_3.baslangicDakika);

  preferences.putInt("r4_cu_v1_ba_s", role_4.cuma.vakit_1.baslangicSaat);
  preferences.putInt("r4_cu_v2_ba_s", role_4.cuma.vakit_2.baslangicSaat);
  preferences.putInt("r4_cu_v3_ba_s", role_4.cuma.vakit_3.baslangicSaat);

  preferences.putInt("r4_cu_v1_ba_d", role_4.cuma.vakit_1.baslangicDakika);
  preferences.putInt("r4_cu_v2_ba_d", role_4.cuma.vakit_2.baslangicDakika);
  preferences.putInt("r4_cu_v3_ba_d", role_4.cuma.vakit_3.baslangicDakika);

  preferences.putInt("r4_ct_v1_ba_s", role_4.cumartesi.vakit_1.baslangicSaat);
  preferences.putInt("r4_ct_v2_ba_s", role_4.cumartesi.vakit_2.baslangicSaat);
  preferences.putInt("r4_ct_v3_ba_s", role_4.cumartesi.vakit_3.baslangicSaat);

  preferences.putInt("r4_ct_v1_ba_d", role_4.cumartesi.vakit_1.baslangicDakika);
  preferences.putInt("r4_ct_v2_ba_d", role_4.cumartesi.vakit_2.baslangicDakika);
  preferences.putInt("r4_ct_v3_ba_d", role_4.cumartesi.vakit_3.baslangicDakika);

  preferences.putInt("r4_pz_v1_ba_s", role_4.pazar.vakit_1.baslangicSaat);
  preferences.putInt("r4_pz_v2_ba_s", role_4.pazar.vakit_2.baslangicSaat);
  preferences.putInt("r4_pz_v3_ba_s", role_4.pazar.vakit_3.baslangicSaat);

  preferences.putInt("r4_pz_v1_ba_d", role_4.pazar.vakit_1.baslangicDakika);
  preferences.putInt("r4_pz_v2_ba_d", role_4.pazar.vakit_2.baslangicDakika);
  preferences.putInt("r4_pz_v3_ba_d", role_4.pazar.vakit_3.baslangicDakika);
}

void kaydetRole1Bitis()
{
  preferences.putInt("r1_pt_v1_bi_s", role_1.pazartesi.vakit_1.bitisSaat);
  preferences.putInt("r1_pt_v2_bi_s", role_1.pazartesi.vakit_2.bitisSaat);
  preferences.putInt("r1_pt_v3_bi_s", role_1.pazartesi.vakit_3.bitisSaat);

  preferences.putInt("r1_pt_v1_bi_d", role_1.pazartesi.vakit_1.bitisDakika);
  preferences.putInt("r1_pt_v2_bi_d", role_1.pazartesi.vakit_2.bitisDakika);
  preferences.putInt("r1_pt_v3_bi_d", role_1.pazartesi.vakit_3.bitisDakika);

  preferences.putInt("r1_sl_v1_bi_s", role_1.sali.vakit_1.bitisSaat);
  preferences.putInt("r1_sl_v2_bi_s", role_1.sali.vakit_2.bitisSaat);
  preferences.putInt("r1_sl_v3_bi_s", role_1.sali.vakit_3.bitisSaat);

  preferences.putInt("r1_sl_v1_bi_d", role_1.sali.vakit_1.bitisDakika);
  preferences.putInt("r1_sl_v2_bi_d", role_1.sali.vakit_2.bitisDakika);
  preferences.putInt("r1_sl_v3_bi_d", role_1.sali.vakit_3.bitisDakika);

  preferences.putInt("r1_cr_v1_bi_s", role_1.carsamba.vakit_1.bitisSaat);
  preferences.putInt("r1_cr_v2_bi_s", role_1.carsamba.vakit_2.bitisSaat);
  preferences.putInt("r1_cr_v3_bi_s", role_1.carsamba.vakit_3.bitisSaat);

  preferences.putInt("r1_cr_v1_bi_d", role_1.carsamba.vakit_1.bitisDakika);
  preferences.putInt("r1_cr_v2_bi_d", role_1.carsamba.vakit_2.bitisDakika);
  preferences.putInt("r1_cr_v3_bi_d", role_1.carsamba.vakit_3.bitisDakika);

  preferences.putInt("r1_pr_v1_bi_s", role_1.persembe.vakit_1.bitisSaat);
  preferences.putInt("r1_pr_v2_bi_s", role_1.persembe.vakit_2.bitisSaat);
  preferences.putInt("r1_pr_v3_bi_s", role_1.persembe.vakit_3.bitisSaat);

  preferences.putInt("r1_pr_v1_bi_d", role_1.persembe.vakit_1.bitisDakika);
  preferences.putInt("r1_pr_v2_bi_d", role_1.persembe.vakit_2.bitisDakika);
  preferences.putInt("r1_pr_v3_bi_d", role_1.persembe.vakit_3.bitisDakika);

  preferences.putInt("r1_cu_v1_bi_s", role_1.cuma.vakit_1.bitisSaat);
  preferences.putInt("r1_cu_v2_bi_s", role_1.cuma.vakit_2.bitisSaat);
  preferences.putInt("r1_cu_v3_bi_s", role_1.cuma.vakit_3.bitisSaat);

  preferences.putInt("r1_cu_v1_bi_d", role_1.cuma.vakit_1.bitisDakika);
  preferences.putInt("r1_cu_v2_bi_d", role_1.cuma.vakit_2.bitisDakika);
  preferences.putInt("r1_cu_v3_bi_d", role_1.cuma.vakit_3.bitisDakika);

  preferences.putInt("r1_ct_v1_bi_s", role_1.cumartesi.vakit_1.bitisSaat);
  preferences.putInt("r1_ct_v2_bi_s", role_1.cumartesi.vakit_2.bitisSaat);
  preferences.putInt("r1_ct_v3_bi_s", role_1.cumartesi.vakit_3.bitisSaat);

  preferences.putInt("r1_ct_v1_bi_d", role_1.cumartesi.vakit_1.bitisDakika);
  preferences.putInt("r1_ct_v2_bi_d", role_1.cumartesi.vakit_2.bitisDakika);
  preferences.putInt("r1_ct_v3_bi_d", role_1.cumartesi.vakit_3.bitisDakika);

  preferences.putInt("r1_pz_v1_bi_s", role_1.pazar.vakit_1.bitisSaat);
  preferences.putInt("r1_pz_v2_bi_s", role_1.pazar.vakit_2.bitisSaat);
  preferences.putInt("r1_pz_v3_bi_s", role_1.pazar.vakit_3.bitisSaat);

  preferences.putInt("r1_pz_v1_bi_d", role_1.pazar.vakit_1.bitisDakika);
  preferences.putInt("r1_pz_v2_bi_d", role_1.pazar.vakit_2.bitisDakika);
  preferences.putInt("r1_pz_v3_bi_d", role_1.pazar.vakit_3.bitisDakika);
}

void kaydetRole2Bitis()
{
  preferences.putInt("r2_pt_v1_bi_s", role_2.pazartesi.vakit_1.bitisSaat);
  preferences.putInt("r2_pt_v2_bi_s", role_2.pazartesi.vakit_2.bitisSaat);
  preferences.putInt("r2_pt_v3_bi_s", role_2.pazartesi.vakit_3.bitisSaat);

  preferences.putInt("r2_pt_v1_bi_d", role_2.pazartesi.vakit_1.bitisDakika);
  preferences.putInt("r2_pt_v2_bi_d", role_2.pazartesi.vakit_2.bitisDakika);
  preferences.putInt("r2_pt_v3_bi_d", role_2.pazartesi.vakit_3.bitisDakika);

  preferences.putInt("r2_sl_v1_bi_s", role_2.sali.vakit_1.bitisSaat);
  preferences.putInt("r2_sl_v2_bi_s", role_2.sali.vakit_2.bitisSaat);
  preferences.putInt("r2_sl_v3_bi_s", role_2.sali.vakit_3.bitisSaat);

  preferences.putInt("r2_sl_v1_bi_d", role_2.sali.vakit_1.bitisDakika);
  preferences.putInt("r2_sl_v2_bi_d", role_2.sali.vakit_2.bitisDakika);
  preferences.putInt("r2_sl_v3_bi_d", role_2.sali.vakit_3.bitisDakika);

  preferences.putInt("r2_cr_v1_bi_s", role_2.carsamba.vakit_1.bitisSaat);
  preferences.putInt("r2_cr_v2_bi_s", role_2.carsamba.vakit_2.bitisSaat);
  preferences.putInt("r2_cr_v3_bi_s", role_2.carsamba.vakit_3.bitisSaat);

  preferences.putInt("r2_cr_v1_bi_d", role_2.carsamba.vakit_1.bitisDakika);
  preferences.putInt("r2_cr_v2_bi_d", role_2.carsamba.vakit_2.bitisDakika);
  preferences.putInt("r2_cr_v3_bi_d", role_2.carsamba.vakit_3.bitisDakika);

  preferences.putInt("r2_pr_v1_bi_s", role_2.persembe.vakit_1.bitisSaat);
  preferences.putInt("r2_pr_v2_bi_s", role_2.persembe.vakit_2.bitisSaat);
  preferences.putInt("r2_pr_v3_bi_s", role_2.persembe.vakit_3.bitisSaat);

  preferences.putInt("r2_pr_v1_bi_d", role_2.persembe.vakit_1.bitisDakika);
  preferences.putInt("r2_pr_v2_bi_d", role_2.persembe.vakit_2.bitisDakika);
  preferences.putInt("r2_pr_v3_bi_d", role_2.persembe.vakit_3.bitisDakika);

  preferences.putInt("r2_cu_v1_bi_s", role_2.cuma.vakit_1.bitisSaat);
  preferences.putInt("r2_cu_v2_bi_s", role_2.cuma.vakit_2.bitisSaat);
  preferences.putInt("r2_cu_v3_bi_s", role_2.cuma.vakit_3.bitisSaat);

  preferences.putInt("r2_cu_v1_bi_d", role_2.cuma.vakit_1.bitisDakika);
  preferences.putInt("r2_cu_v2_bi_d", role_2.cuma.vakit_2.bitisDakika);
  preferences.putInt("r2_cu_v3_bi_d", role_2.cuma.vakit_3.bitisDakika);

  preferences.putInt("r2_ct_v1_bi_s", role_2.cumartesi.vakit_1.bitisSaat);
  preferences.putInt("r2_ct_v2_bi_s", role_2.cumartesi.vakit_2.bitisSaat);
  preferences.putInt("r2_ct_v3_bi_s", role_2.cumartesi.vakit_3.bitisSaat);

  preferences.putInt("r2_ct_v1_bi_d", role_2.cumartesi.vakit_1.bitisDakika);
  preferences.putInt("r2_ct_v2_bi_d", role_2.cumartesi.vakit_2.bitisDakika);
  preferences.putInt("r2_ct_v3_bi_d", role_2.cumartesi.vakit_3.bitisDakika);

  preferences.putInt("r2_pz_v1_bi_s", role_2.pazar.vakit_1.bitisSaat);
  preferences.putInt("r2_pz_v2_bi_s", role_2.pazar.vakit_2.bitisSaat);
  preferences.putInt("r2_pz_v3_bi_s", role_2.pazar.vakit_3.bitisSaat);

  preferences.putInt("r2_pz_v1_bi_d", role_2.pazar.vakit_1.bitisDakika);
  preferences.putInt("r2_pz_v2_bi_d", role_2.pazar.vakit_2.bitisDakika);
  preferences.putInt("r2_pz_v3_bi_d", role_2.pazar.vakit_3.bitisDakika);
}

void kaydetRole3Bitis()
{
  preferences.putInt("r3_pt_v1_bi_s", role_3.pazartesi.vakit_1.bitisSaat);
  preferences.putInt("r3_pt_v2_bi_s", role_3.pazartesi.vakit_2.bitisSaat);
  preferences.putInt("r3_pt_v3_bi_s", role_3.pazartesi.vakit_3.bitisSaat);

  preferences.putInt("r3_pt_v1_bi_d", role_3.pazartesi.vakit_1.bitisDakika);
  preferences.putInt("r3_pt_v2_bi_d", role_3.pazartesi.vakit_2.bitisDakika);
  preferences.putInt("r3_pt_v3_bi_d", role_3.pazartesi.vakit_3.bitisDakika);

  preferences.putInt("r3_sl_v1_bi_s", role_3.sali.vakit_1.bitisSaat);
  preferences.putInt("r3_sl_v2_bi_s", role_3.sali.vakit_2.bitisSaat);
  preferences.putInt("r3_sl_v3_bi_s", role_3.sali.vakit_3.bitisSaat);

  preferences.putInt("r3_sl_v1_bi_d", role_3.sali.vakit_1.bitisDakika);
  preferences.putInt("r3_sl_v2_bi_d", role_3.sali.vakit_2.bitisDakika);
  preferences.putInt("r3_sl_v3_bi_d", role_3.sali.vakit_3.bitisDakika);

  preferences.putInt("r3_cr_v1_bi_s", role_3.carsamba.vakit_1.bitisSaat);
  preferences.putInt("r3_cr_v2_bi_s", role_3.carsamba.vakit_2.bitisSaat);
  preferences.putInt("r3_cr_v3_bi_s", role_3.carsamba.vakit_3.bitisSaat);

  preferences.putInt("r3_cr_v1_bi_d", role_3.carsamba.vakit_1.bitisDakika);
  preferences.putInt("r3_cr_v2_bi_d", role_3.carsamba.vakit_2.bitisDakika);
  preferences.putInt("r3_cr_v3_bi_d", role_3.carsamba.vakit_3.bitisDakika);

  preferences.putInt("r3_pr_v1_bi_s", role_3.persembe.vakit_1.bitisSaat);
  preferences.putInt("r3_pr_v2_bi_s", role_3.persembe.vakit_2.bitisSaat);
  preferences.putInt("r3_pr_v3_bi_s", role_3.persembe.vakit_3.bitisSaat);

  preferences.putInt("r3_pr_v1_bi_d", role_3.persembe.vakit_1.bitisDakika);
  preferences.putInt("r3_pr_v2_bi_d", role_3.persembe.vakit_2.bitisDakika);
  preferences.putInt("r3_pr_v3_bi_d", role_3.persembe.vakit_3.bitisDakika);

  preferences.putInt("r3_cu_v1_bi_s", role_3.cuma.vakit_1.bitisSaat);
  preferences.putInt("r3_cu_v2_bi_s", role_3.cuma.vakit_2.bitisSaat);
  preferences.putInt("r3_cu_v3_bi_s", role_3.cuma.vakit_3.bitisSaat);

  preferences.putInt("r3_cu_v1_bi_d", role_3.cuma.vakit_1.bitisDakika);
  preferences.putInt("r3_cu_v2_bi_d", role_3.cuma.vakit_2.bitisDakika);
  preferences.putInt("r3_cu_v3_bi_d", role_3.cuma.vakit_3.bitisDakika);

  preferences.putInt("r3_ct_v1_bi_s", role_3.cumartesi.vakit_1.bitisSaat);
  preferences.putInt("r3_ct_v2_bi_s", role_3.cumartesi.vakit_2.bitisSaat);
  preferences.putInt("r3_ct_v3_bi_s", role_3.cumartesi.vakit_3.bitisSaat);

  preferences.putInt("r3_ct_v1_bi_d", role_3.cumartesi.vakit_1.bitisDakika);
  preferences.putInt("r3_ct_v2_bi_d", role_3.cumartesi.vakit_2.bitisDakika);
  preferences.putInt("r3_ct_v3_bi_d", role_3.cumartesi.vakit_3.bitisDakika);

  preferences.putInt("r3_pz_v1_bi_s", role_3.pazar.vakit_1.bitisSaat);
  preferences.putInt("r3_pz_v2_bi_s", role_3.pazar.vakit_2.bitisSaat);
  preferences.putInt("r3_pz_v3_bi_s", role_3.pazar.vakit_3.bitisSaat);

  preferences.putInt("r3_pz_v1_bi_d", role_3.pazar.vakit_1.bitisDakika);
  preferences.putInt("r3_pz_v2_bi_d", role_3.pazar.vakit_2.bitisDakika);
  preferences.putInt("r3_pz_v3_bi_d", role_3.pazar.vakit_3.bitisDakika);
}

void kaydetRole4Bitis()
{
  preferences.putInt("r4_pt_v1_bi_s", role_4.pazartesi.vakit_1.bitisSaat);
  preferences.putInt("r4_pt_v2_bi_s", role_4.pazartesi.vakit_2.bitisSaat);
  preferences.putInt("r4_pt_v3_bi_s", role_4.pazartesi.vakit_3.bitisSaat);

  preferences.putInt("r4_pt_v1_bi_d", role_4.pazartesi.vakit_1.bitisDakika);
  preferences.putInt("r4_pt_v2_bi_d", role_4.pazartesi.vakit_2.bitisDakika);
  preferences.putInt("r4_pt_v3_bi_d", role_4.pazartesi.vakit_3.bitisDakika);

  preferences.putInt("r4_sl_v1_bi_s", role_4.sali.vakit_1.bitisSaat);
  preferences.putInt("r4_sl_v2_bi_s", role_4.sali.vakit_2.bitisSaat);
  preferences.putInt("r4_sl_v3_bi_s", role_4.sali.vakit_3.bitisSaat);

  preferences.putInt("r4_sl_v1_bi_d", role_4.sali.vakit_1.bitisDakika);
  preferences.putInt("r4_sl_v2_bi_d", role_4.sali.vakit_2.bitisDakika);
  preferences.putInt("r4_sl_v3_bi_d", role_4.sali.vakit_3.bitisDakika);

  preferences.putInt("r4_cr_v1_bi_s", role_4.carsamba.vakit_1.bitisSaat);
  preferences.putInt("r4_cr_v2_bi_s", role_4.carsamba.vakit_2.bitisSaat);
  preferences.putInt("r4_cr_v3_bi_s", role_4.carsamba.vakit_3.bitisSaat);

  preferences.putInt("r4_cr_v1_bi_d", role_4.carsamba.vakit_1.bitisDakika);
  preferences.putInt("r4_cr_v2_bi_d", role_4.carsamba.vakit_2.bitisDakika);
  preferences.putInt("r4_cr_v3_bi_d", role_4.carsamba.vakit_3.bitisDakika);

  preferences.putInt("r4_pr_v1_bi_s", role_4.persembe.vakit_1.bitisSaat);
  preferences.putInt("r4_pr_v2_bi_s", role_4.persembe.vakit_2.bitisSaat);
  preferences.putInt("r4_pr_v3_bi_s", role_4.persembe.vakit_3.bitisSaat);

  preferences.putInt("r4_pr_v1_bi_d", role_4.persembe.vakit_1.bitisDakika);
  preferences.putInt("r4_pr_v2_bi_d", role_4.persembe.vakit_2.bitisDakika);
  preferences.putInt("r4_pr_v3_bi_d", role_4.persembe.vakit_3.bitisDakika);

  preferences.putInt("r4_cu_v1_bi_s", role_4.cuma.vakit_1.bitisSaat);
  preferences.putInt("r4_cu_v2_bi_s", role_4.cuma.vakit_2.bitisSaat);
  preferences.putInt("r4_cu_v3_bi_s", role_4.cuma.vakit_3.bitisSaat);

  preferences.putInt("r4_cu_v1_bi_d", role_4.cuma.vakit_1.bitisDakika);
  preferences.putInt("r4_cu_v2_bi_d", role_4.cuma.vakit_2.bitisDakika);
  preferences.putInt("r4_cu_v3_bi_d", role_4.cuma.vakit_3.bitisDakika);

  preferences.putInt("r4_ct_v1_bi_s", role_4.cumartesi.vakit_1.bitisSaat);
  preferences.putInt("r4_ct_v2_bi_s", role_4.cumartesi.vakit_2.bitisSaat);
  preferences.putInt("r4_ct_v3_bi_s", role_4.cumartesi.vakit_3.bitisSaat);

  preferences.putInt("r4_ct_v1_bi_d", role_4.cumartesi.vakit_1.bitisDakika);
  preferences.putInt("r4_ct_v2_bi_d", role_4.cumartesi.vakit_2.bitisDakika);
  preferences.putInt("r4_ct_v3_bi_d", role_4.cumartesi.vakit_3.bitisDakika);

  preferences.putInt("r4_pz_v1_bi_s", role_4.pazar.vakit_1.bitisSaat);
  preferences.putInt("r4_pz_v2_bi_s", role_4.pazar.vakit_2.bitisSaat);
  preferences.putInt("r4_pz_v3_bi_s", role_4.pazar.vakit_3.bitisSaat);

  preferences.putInt("r4_pz_v1_bi_d", role_4.pazar.vakit_1.bitisDakika);
  preferences.putInt("r4_pz_v2_bi_d", role_4.pazar.vakit_2.bitisDakika);
  preferences.putInt("r4_pz_v3_bi_d", role_4.pazar.vakit_3.bitisDakika);
}

void verileriCekRole1Baslangic()
{
  role_1.pazartesi.vakit_1.baslangicSaat = preferences.getInt("r1_pt_v1_ba_s", false);
  role_1.pazartesi.vakit_2.baslangicSaat = preferences.getInt("r1_pt_v2_ba_s", false);
  role_1.pazartesi.vakit_3.baslangicSaat = preferences.getInt("r1_pt_v3_ba_s", false);

  role_1.pazartesi.vakit_1.baslangicDakika = preferences.getInt("r1_pt_v1_ba_d", false);
  role_1.pazartesi.vakit_2.baslangicDakika = preferences.getInt("r1_pt_v2_ba_d", false);
  role_1.pazartesi.vakit_3.baslangicDakika = preferences.getInt("r1_pt_v3_ba_d", false);

  role_1.sali.vakit_1.baslangicSaat = preferences.getInt("r1_sl_v1_ba_s", false);
  role_1.sali.vakit_2.baslangicSaat = preferences.getInt("r1_sl_v2_ba_s", false);
  role_1.sali.vakit_3.baslangicSaat = preferences.getInt("r1_sl_v3_ba_s", false);

  role_1.sali.vakit_1.baslangicDakika = preferences.getInt("r1_sl_v1_ba_d", false);
  role_1.sali.vakit_2.baslangicDakika = preferences.getInt("r1_sl_v2_ba_d", false);
  role_1.sali.vakit_3.baslangicDakika = preferences.getInt("r1_sl_v3_ba_d", false);

  role_1.carsamba.vakit_1.baslangicSaat = preferences.getInt("r1_cr_v1_ba_s", false);
  role_1.carsamba.vakit_2.baslangicSaat = preferences.getInt("r1_cr_v2_ba_s", false);
  role_1.carsamba.vakit_3.baslangicSaat = preferences.getInt("r1_cr_v3_ba_s", false);

  role_1.carsamba.vakit_1.baslangicDakika = preferences.getInt("r1_cr_v1_ba_d", false);
  role_1.carsamba.vakit_2.baslangicDakika = preferences.getInt("r1_cr_v2_ba_d", false);
  role_1.carsamba.vakit_3.baslangicDakika = preferences.getInt("r1_cr_v3_ba_d", false);

  role_1.persembe.vakit_1.baslangicSaat = preferences.getInt("r1_pr_v1_ba_s", false);
  role_1.persembe.vakit_2.baslangicSaat = preferences.getInt("r1_pr_v2_ba_s", false);
  role_1.persembe.vakit_3.baslangicSaat = preferences.getInt("r1_pr_v3_ba_s", false);

  role_1.persembe.vakit_1.baslangicDakika = preferences.getInt("r1_pr_v1_ba_d", false);
  role_1.persembe.vakit_2.baslangicDakika = preferences.getInt("r1_pr_v2_ba_d", false);
  role_1.persembe.vakit_3.baslangicDakika = preferences.getInt("r1_pr_v3_ba_d", false);

  role_1.cuma.vakit_1.baslangicSaat = preferences.getInt("r1_cu_v1_ba_s", false);
  role_1.cuma.vakit_2.baslangicSaat = preferences.getInt("r1_cu_v2_ba_s", false);
  role_1.cuma.vakit_3.baslangicSaat = preferences.getInt("r1_cu_v3_ba_s", false);

  role_1.cuma.vakit_1.baslangicDakika = preferences.getInt("r1_cu_v1_ba_d", false);
  role_1.cuma.vakit_2.baslangicDakika = preferences.getInt("r1_cu_v2_ba_d", false);
  role_1.cuma.vakit_3.baslangicDakika = preferences.getInt("r1_cu_v3_ba_d", false);

  role_1.cumartesi.vakit_1.baslangicSaat = preferences.getInt("r1_ct_v1_ba_s", false);
  role_1.cumartesi.vakit_2.baslangicSaat = preferences.getInt("r1_ct_v2_ba_s", false);
  role_1.cumartesi.vakit_3.baslangicSaat = preferences.getInt("r1_ct_v3_ba_s", false);

  role_1.cumartesi.vakit_1.baslangicDakika = preferences.getInt("r1_ct_v1_ba_d", false);
  role_1.cumartesi.vakit_2.baslangicDakika = preferences.getInt("r1_ct_v2_ba_d", false);
  role_1.cumartesi.vakit_3.baslangicDakika = preferences.getInt("r1_ct_v3_ba_d", false);

  role_1.pazar.vakit_1.baslangicSaat = preferences.getInt("r1_pz_v1_ba_s", false);
  role_1.pazar.vakit_2.baslangicSaat = preferences.getInt("r1_pz_v2_ba_s", false);
  role_1.pazar.vakit_3.baslangicSaat = preferences.getInt("r1_pz_v3_ba_s", false);

  role_1.pazar.vakit_1.baslangicDakika = preferences.getInt("r1_pz_v1_ba_d", false);
  role_1.pazar.vakit_2.baslangicDakika = preferences.getInt("r1_pz_v2_ba_d", false);
  role_1.pazar.vakit_3.baslangicDakika = preferences.getInt("r1_pz_v3_ba_d", false);
}

void verileriCekRole1Bitis()
{
  role_1.pazartesi.vakit_1.bitisSaat = preferences.getInt("r1_pt_v1_bi_s", false);
  role_1.pazartesi.vakit_2.bitisSaat = preferences.getInt("r1_pt_v2_bi_s", false);
  role_1.pazartesi.vakit_3.bitisSaat = preferences.getInt("r1_pt_v3_bi_s", false);

  role_1.pazartesi.vakit_1.bitisDakika = preferences.getInt("r1_pt_v1_bi_d", false);
  role_1.pazartesi.vakit_2.bitisDakika = preferences.getInt("r1_pt_v2_bi_d", false);
  role_1.pazartesi.vakit_3.bitisDakika = preferences.getInt("r1_pt_v3_bi_d", false);

  role_1.sali.vakit_1.bitisSaat = preferences.getInt("r1_sl_v1_bi_s", false);
  role_1.sali.vakit_2.bitisSaat = preferences.getInt("r1_sl_v2_bi_s", false);
  role_1.sali.vakit_3.bitisSaat = preferences.getInt("r1_sl_v3_bi_s", false);

  role_1.sali.vakit_1.bitisDakika = preferences.getInt("r1_sl_v1_bi_d", false);
  role_1.sali.vakit_2.bitisDakika = preferences.getInt("r1_sl_v2_bi_d", false);
  role_1.sali.vakit_3.bitisDakika = preferences.getInt("r1_sl_v3_bi_d", false);

  role_1.carsamba.vakit_1.bitisSaat = preferences.getInt("r1_cr_v1_bi_s", false);
  role_1.carsamba.vakit_2.bitisSaat = preferences.getInt("r1_cr_v2_bi_s", false);
  role_1.carsamba.vakit_3.bitisSaat = preferences.getInt("r1_cr_v3_bi_s", false);

  role_1.carsamba.vakit_1.bitisDakika = preferences.getInt("r1_cr_v1_bi_d", false);
  role_1.carsamba.vakit_2.bitisDakika = preferences.getInt("r1_cr_v2_bi_d", false);
  role_1.carsamba.vakit_3.bitisDakika = preferences.getInt("r1_cr_v3_bi_d", false);

  role_1.persembe.vakit_1.bitisSaat = preferences.getInt("r1_pr_v1_bi_s", false);
  role_1.persembe.vakit_2.bitisSaat = preferences.getInt("r1_pr_v2_bi_s", false);
  role_1.persembe.vakit_3.bitisSaat = preferences.getInt("r1_pr_v3_bi_s", false);

  role_1.persembe.vakit_1.bitisDakika = preferences.getInt("r1_pr_v1_bi_d", false);
  role_1.persembe.vakit_2.bitisDakika = preferences.getInt("r1_pr_v2_bi_d", false);
  role_1.persembe.vakit_3.bitisDakika = preferences.getInt("r1_pr_v3_bi_d", false);

  role_1.cuma.vakit_1.bitisSaat = preferences.getInt("r1_cu_v1_bi_s", false);
  role_1.cuma.vakit_2.bitisSaat = preferences.getInt("r1_cu_v2_bi_s", false);
  role_1.cuma.vakit_3.bitisSaat = preferences.getInt("r1_cu_v3_bi_s", false);

  role_1.cuma.vakit_1.bitisDakika = preferences.getInt("r1_cu_v1_bi_d", false);
  role_1.cuma.vakit_2.bitisDakika = preferences.getInt("r1_cu_v2_bi_d", false);
  role_1.cuma.vakit_3.bitisDakika = preferences.getInt("r1_cu_v3_bi_d", false);

  role_1.cumartesi.vakit_1.bitisSaat = preferences.getInt("r1_ct_v1_bi_s", false);
  role_1.cumartesi.vakit_2.bitisSaat = preferences.getInt("r1_ct_v2_bi_s", false);
  role_1.cumartesi.vakit_3.bitisSaat = preferences.getInt("r1_ct_v3_bi_s", false);

  role_1.cumartesi.vakit_1.bitisDakika = preferences.getInt("r1_ct_v1_bi_d", false);
  role_1.cumartesi.vakit_2.bitisDakika = preferences.getInt("r1_ct_v2_bi_d", false);
  role_1.cumartesi.vakit_3.bitisDakika = preferences.getInt("r1_ct_v3_bi_d", false);

  role_1.pazar.vakit_1.bitisSaat = preferences.getInt("r1_pz_v1_bi_s", false);
  role_1.pazar.vakit_2.bitisSaat = preferences.getInt("r1_pz_v2_bi_s", false);
  role_1.pazar.vakit_3.bitisSaat = preferences.getInt("r1_pz_v3_bi_s", false);

  role_1.pazar.vakit_1.bitisDakika = preferences.getInt("r1_pz_v1_bi_d", false);
  role_1.pazar.vakit_2.bitisDakika = preferences.getInt("r1_pz_v2_bi_d", false);
  role_1.pazar.vakit_3.bitisDakika = preferences.getInt("r1_pz_v3_bi_d", false);
}

void verileriCekRole2Baslangic()
{
  role_2.pazartesi.vakit_1.baslangicSaat = preferences.getInt("r2_pt_v1_ba_s", false);
  role_2.pazartesi.vakit_2.baslangicSaat = preferences.getInt("r2_pt_v2_ba_s", false);
  role_2.pazartesi.vakit_3.baslangicSaat = preferences.getInt("r2_pt_v3_ba_s", false);

  role_2.pazartesi.vakit_1.baslangicDakika = preferences.getInt("r2_pt_v1_ba_d", false);
  role_2.pazartesi.vakit_2.baslangicDakika = preferences.getInt("r2_pt_v2_ba_d", false);
  role_2.pazartesi.vakit_3.baslangicDakika = preferences.getInt("r2_pt_v3_ba_d", false);

  role_2.sali.vakit_1.baslangicSaat = preferences.getInt("r2_sl_v1_ba_s", false);
  role_2.sali.vakit_2.baslangicSaat = preferences.getInt("r2_sl_v2_ba_s", false);
  role_2.sali.vakit_3.baslangicSaat = preferences.getInt("r2_sl_v3_ba_s", false);

  role_2.sali.vakit_1.baslangicDakika = preferences.getInt("r2_sl_v1_ba_d", false);
  role_2.sali.vakit_2.baslangicDakika = preferences.getInt("r2_sl_v2_ba_d", false);
  role_2.sali.vakit_3.baslangicDakika = preferences.getInt("r2_sl_v3_ba_d", false);

  role_2.carsamba.vakit_1.baslangicSaat = preferences.getInt("r2_cr_v1_ba_s", false);
  role_2.carsamba.vakit_2.baslangicSaat = preferences.getInt("r2_cr_v2_ba_s", false);
  role_2.carsamba.vakit_3.baslangicSaat = preferences.getInt("r2_cr_v3_ba_s", false);

  role_2.carsamba.vakit_1.baslangicDakika = preferences.getInt("r2_cr_v1_ba_d", false);
  role_2.carsamba.vakit_2.baslangicDakika = preferences.getInt("r2_cr_v2_ba_d", false);
  role_2.carsamba.vakit_3.baslangicDakika = preferences.getInt("r2_cr_v3_ba_d", false);

  role_2.persembe.vakit_1.baslangicSaat = preferences.getInt("r2_pr_v1_ba_s", false);
  role_2.persembe.vakit_2.baslangicSaat = preferences.getInt("r2_pr_v2_ba_s", false);
  role_2.persembe.vakit_3.baslangicSaat = preferences.getInt("r2_pr_v3_ba_s", false);

  role_2.persembe.vakit_1.baslangicDakika = preferences.getInt("r2_pr_v1_ba_d", false);
  role_2.persembe.vakit_2.baslangicDakika = preferences.getInt("r2_pr_v2_ba_d", false);
  role_2.persembe.vakit_3.baslangicDakika = preferences.getInt("r2_pr_v3_ba_d", false);

  role_2.cuma.vakit_1.baslangicSaat = preferences.getInt("r2_cu_v1_ba_s", false);
  role_2.cuma.vakit_2.baslangicSaat = preferences.getInt("r2_cu_v2_ba_s", false);
  role_2.cuma.vakit_3.baslangicSaat = preferences.getInt("r2_cu_v3_ba_s", false);

  role_2.cuma.vakit_1.baslangicDakika = preferences.getInt("r2_cu_v1_ba_d", false);
  role_2.cuma.vakit_2.baslangicDakika = preferences.getInt("r2_cu_v2_ba_d", false);
  role_2.cuma.vakit_3.baslangicDakika = preferences.getInt("r2_cu_v3_ba_d", false);

  role_2.cumartesi.vakit_1.baslangicSaat = preferences.getInt("r2_ct_v1_ba_s", false);
  role_2.cumartesi.vakit_2.baslangicSaat = preferences.getInt("r2_ct_v2_ba_s", false);
  role_2.cumartesi.vakit_3.baslangicSaat = preferences.getInt("r2_ct_v3_ba_s", false);

  role_2.cumartesi.vakit_1.baslangicDakika = preferences.getInt("r2_ct_v1_ba_d", false);
  role_2.cumartesi.vakit_2.baslangicDakika = preferences.getInt("r2_ct_v2_ba_d", false);
  role_2.cumartesi.vakit_3.baslangicDakika = preferences.getInt("r2_ct_v3_ba_d", false);

  role_2.pazar.vakit_1.baslangicSaat = preferences.getInt("r2_pz_v1_ba_s", false);
  role_2.pazar.vakit_2.baslangicSaat = preferences.getInt("r2_pz_v2_ba_s", false);
  role_2.pazar.vakit_3.baslangicSaat = preferences.getInt("r2_pz_v3_ba_s", false);

  role_2.pazar.vakit_1.baslangicDakika = preferences.getInt("r2_pz_v1_ba_d", false);
  role_2.pazar.vakit_2.baslangicDakika = preferences.getInt("r2_pz_v2_ba_d", false);
  role_2.pazar.vakit_3.baslangicDakika = preferences.getInt("r2_pz_v3_ba_d", false);
}

void verileriCekRole2Bitis()
{
  role_2.pazartesi.vakit_1.bitisSaat = preferences.getInt("r2_pt_v1_bi_s", false);
  role_2.pazartesi.vakit_2.bitisSaat = preferences.getInt("r2_pt_v2_bi_s", false);
  role_2.pazartesi.vakit_3.bitisSaat = preferences.getInt("r2_pt_v3_bi_s", false);

  role_2.pazartesi.vakit_1.bitisDakika = preferences.getInt("r2_pt_v1_bi_d", false);
  role_2.pazartesi.vakit_2.bitisDakika = preferences.getInt("r2_pt_v2_bi_d", false);
  role_2.pazartesi.vakit_3.bitisDakika = preferences.getInt("r2_pt_v3_bi_d", false);

  role_2.sali.vakit_1.bitisSaat = preferences.getInt("r2_sl_v1_bi_s", false);
  role_2.sali.vakit_2.bitisSaat = preferences.getInt("r2_sl_v2_bi_s", false);
  role_2.sali.vakit_3.bitisSaat = preferences.getInt("r2_sl_v3_bi_s", false);

  role_2.sali.vakit_1.bitisDakika = preferences.getInt("r2_sl_v1_bi_d", false);
  role_2.sali.vakit_2.bitisDakika = preferences.getInt("r2_sl_v2_bi_d", false);
  role_2.sali.vakit_3.bitisDakika = preferences.getInt("r2_sl_v3_bi_d", false);

  role_2.carsamba.vakit_1.bitisSaat = preferences.getInt("r2_cr_v1_bi_s", false);
  role_2.carsamba.vakit_2.bitisSaat = preferences.getInt("r2_cr_v2_bi_s", false);
  role_2.carsamba.vakit_3.bitisSaat = preferences.getInt("r2_cr_v3_bi_s", false);

  role_2.carsamba.vakit_1.bitisDakika = preferences.getInt("r2_cr_v1_bi_d", false);
  role_2.carsamba.vakit_2.bitisDakika = preferences.getInt("r2_cr_v2_bi_d", false);
  role_2.carsamba.vakit_3.bitisDakika = preferences.getInt("r2_cr_v3_bi_d", false);

  role_2.persembe.vakit_1.bitisSaat = preferences.getInt("r2_pr_v1_bi_s", false);
  role_2.persembe.vakit_2.bitisSaat = preferences.getInt("r2_pr_v2_bi_s", false);
  role_2.persembe.vakit_3.bitisSaat = preferences.getInt("r2_pr_v3_bi_s", false);

  role_2.persembe.vakit_1.bitisDakika = preferences.getInt("r2_pr_v1_bi_d", false);
  role_2.persembe.vakit_2.bitisDakika = preferences.getInt("r2_pr_v2_bi_d", false);
  role_2.persembe.vakit_3.bitisDakika = preferences.getInt("r2_pr_v3_bi_d", false);

  role_2.cuma.vakit_1.bitisSaat = preferences.getInt("r2_cu_v1_bi_s", false);
  role_2.cuma.vakit_2.bitisSaat = preferences.getInt("r2_cu_v2_bi_s", false);
  role_2.cuma.vakit_3.bitisSaat = preferences.getInt("r2_cu_v3_bi_s", false);

  role_2.cuma.vakit_1.bitisDakika = preferences.getInt("r2_cu_v1_bi_d", false);
  role_2.cuma.vakit_2.bitisDakika = preferences.getInt("r2_cu_v2_bi_d", false);
  role_2.cuma.vakit_3.bitisDakika = preferences.getInt("r2_cu_v3_bi_d", false);

  role_2.cumartesi.vakit_1.bitisSaat = preferences.getInt("r2_ct_v1_bi_s", false);
  role_2.cumartesi.vakit_2.bitisSaat = preferences.getInt("r2_ct_v2_bi_s", false);
  role_2.cumartesi.vakit_3.bitisSaat = preferences.getInt("r2_ct_v3_bi_s", false);

  role_2.cumartesi.vakit_1.bitisDakika = preferences.getInt("r2_ct_v1_bi_d", false);
  role_2.cumartesi.vakit_2.bitisDakika = preferences.getInt("r2_ct_v2_bi_d", false);
  role_2.cumartesi.vakit_3.bitisDakika = preferences.getInt("r2_ct_v3_bi_d", false);

  role_2.pazar.vakit_1.bitisSaat = preferences.getInt("r2_pz_v1_bi_s", false);
  role_2.pazar.vakit_2.bitisSaat = preferences.getInt("r2_pz_v2_bi_s", false);
  role_2.pazar.vakit_3.bitisSaat = preferences.getInt("r2_pz_v3_bi_s", false);

  role_2.pazar.vakit_1.bitisDakika = preferences.getInt("r2_pz_v1_bi_d", false);
  role_2.pazar.vakit_2.bitisDakika = preferences.getInt("r2_pz_v2_bi_d", false);
  role_2.pazar.vakit_3.bitisDakika = preferences.getInt("r2_pz_v3_bi_d", false);
}

void verileriCekRole3Baslangic()
{
  role_3.pazartesi.vakit_1.baslangicSaat = preferences.getInt("r3_pt_v1_ba_s", false);
  role_3.pazartesi.vakit_2.baslangicSaat = preferences.getInt("r3_pt_v2_ba_s", false);
  role_3.pazartesi.vakit_3.baslangicSaat = preferences.getInt("r3_pt_v3_ba_s", false);

  role_3.pazartesi.vakit_1.baslangicDakika = preferences.getInt("r3_pt_v1_ba_d", false);
  role_3.pazartesi.vakit_2.baslangicDakika = preferences.getInt("r3_pt_v2_ba_d", false);
  role_3.pazartesi.vakit_3.baslangicDakika = preferences.getInt("r3_pt_v3_ba_d", false);

  role_3.sali.vakit_1.baslangicSaat = preferences.getInt("r3_sl_v1_ba_s", false);
  role_3.sali.vakit_2.baslangicSaat = preferences.getInt("r3_sl_v2_ba_s", false);
  role_3.sali.vakit_3.baslangicSaat = preferences.getInt("r3_sl_v3_ba_s", false);

  role_3.sali.vakit_1.baslangicDakika = preferences.getInt("r3_sl_v1_ba_d", false);
  role_3.sali.vakit_2.baslangicDakika = preferences.getInt("r3_sl_v2_ba_d", false);
  role_3.sali.vakit_3.baslangicDakika = preferences.getInt("r3_sl_v3_ba_d", false);

  role_3.carsamba.vakit_1.baslangicSaat = preferences.getInt("r3_cr_v1_ba_s", false);
  role_3.carsamba.vakit_2.baslangicSaat = preferences.getInt("r3_cr_v2_ba_s", false);
  role_3.carsamba.vakit_3.baslangicSaat = preferences.getInt("r3_cr_v3_ba_s", false);

  role_3.carsamba.vakit_1.baslangicDakika = preferences.getInt("r3_cr_v1_ba_d", false);
  role_3.carsamba.vakit_2.baslangicDakika = preferences.getInt("r3_cr_v2_ba_d", false);
  role_3.carsamba.vakit_3.baslangicDakika = preferences.getInt("r3_cr_v3_ba_d", false);

  role_3.persembe.vakit_1.baslangicSaat = preferences.getInt("r3_pr_v1_ba_s", false);
  role_3.persembe.vakit_2.baslangicSaat = preferences.getInt("r3_pr_v2_ba_s", false);
  role_3.persembe.vakit_3.baslangicSaat = preferences.getInt("r3_pr_v3_ba_s", false);

  role_3.persembe.vakit_1.baslangicDakika = preferences.getInt("r3_pr_v1_ba_d", false);
  role_3.persembe.vakit_2.baslangicDakika = preferences.getInt("r3_pr_v2_ba_d", false);
  role_3.persembe.vakit_3.baslangicDakika = preferences.getInt("r3_pr_v3_ba_d", false);

  role_3.cuma.vakit_1.baslangicSaat = preferences.getInt("r3_cu_v1_ba_s", false);
  role_3.cuma.vakit_2.baslangicSaat = preferences.getInt("r3_cu_v2_ba_s", false);
  role_3.cuma.vakit_3.baslangicSaat = preferences.getInt("r3_cu_v3_ba_s", false);

  role_3.cuma.vakit_1.baslangicDakika = preferences.getInt("r3_cu_v1_ba_d", false);
  role_3.cuma.vakit_2.baslangicDakika = preferences.getInt("r3_cu_v2_ba_d", false);
  role_3.cuma.vakit_3.baslangicDakika = preferences.getInt("r3_cu_v3_ba_d", false);

  role_3.cumartesi.vakit_1.baslangicSaat = preferences.getInt("r3_ct_v1_ba_s", false);
  role_3.cumartesi.vakit_2.baslangicSaat = preferences.getInt("r3_ct_v2_ba_s", false);
  role_3.cumartesi.vakit_3.baslangicSaat = preferences.getInt("r3_ct_v3_ba_s", false);

  role_3.cumartesi.vakit_1.baslangicDakika = preferences.getInt("r3_ct_v1_ba_d", false);
  role_3.cumartesi.vakit_2.baslangicDakika = preferences.getInt("r3_ct_v2_ba_d", false);
  role_3.cumartesi.vakit_3.baslangicDakika = preferences.getInt("r3_ct_v3_ba_d", false);

  role_3.pazar.vakit_1.baslangicSaat = preferences.getInt("r3_pz_v1_ba_s", false);
  role_3.pazar.vakit_2.baslangicSaat = preferences.getInt("r3_pz_v2_ba_s", false);
  role_3.pazar.vakit_3.baslangicSaat = preferences.getInt("r3_pz_v3_ba_s", false);

  role_3.pazar.vakit_1.baslangicDakika = preferences.getInt("r3_pz_v1_ba_d", false);
  role_3.pazar.vakit_2.baslangicDakika = preferences.getInt("r3_pz_v2_ba_d", false);
  role_3.pazar.vakit_3.baslangicDakika = preferences.getInt("r3_pz_v3_ba_d", false);
}

void verileriCekRole3Bitis()
{
  role_3.pazartesi.vakit_1.bitisSaat = preferences.getInt("r3_pt_v1_bi_s", false);
  role_3.pazartesi.vakit_2.bitisSaat = preferences.getInt("r3_pt_v2_bi_s", false);
  role_3.pazartesi.vakit_3.bitisSaat = preferences.getInt("r3_pt_v3_bi_s", false);

  role_3.pazartesi.vakit_1.bitisDakika = preferences.getInt("r3_pt_v1_bi_d", false);
  role_3.pazartesi.vakit_2.bitisDakika = preferences.getInt("r3_pt_v2_bi_d", false);
  role_3.pazartesi.vakit_3.bitisDakika = preferences.getInt("r3_pt_v3_bi_d", false);

  role_3.sali.vakit_1.bitisSaat = preferences.getInt("r3_sl_v1_bi_s", false);
  role_3.sali.vakit_2.bitisSaat = preferences.getInt("r3_sl_v2_bi_s", false);
  role_3.sali.vakit_3.bitisSaat = preferences.getInt("r3_sl_v3_bi_s", false);

  role_3.sali.vakit_1.bitisDakika = preferences.getInt("r3_sl_v1_bi_d", false);
  role_3.sali.vakit_2.bitisDakika = preferences.getInt("r3_sl_v2_bi_d", false);
  role_3.sali.vakit_3.bitisDakika = preferences.getInt("r3_sl_v3_bi_d", false);

  role_3.carsamba.vakit_1.bitisSaat = preferences.getInt("r3_cr_v1_bi_s", false);
  role_3.carsamba.vakit_2.bitisSaat = preferences.getInt("r3_cr_v2_bi_s", false);
  role_3.carsamba.vakit_3.bitisSaat = preferences.getInt("r3_cr_v3_bi_s", false);

  role_3.carsamba.vakit_1.bitisDakika = preferences.getInt("r3_cr_v1_bi_d", false);
  role_3.carsamba.vakit_2.bitisDakika = preferences.getInt("r3_cr_v2_bi_d", false);
  role_3.carsamba.vakit_3.bitisDakika = preferences.getInt("r3_cr_v3_bi_d", false);

  role_3.persembe.vakit_1.bitisSaat = preferences.getInt("r3_pr_v1_bi_s", false);
  role_3.persembe.vakit_2.bitisSaat = preferences.getInt("r3_pr_v2_bi_s", false);
  role_3.persembe.vakit_3.bitisSaat = preferences.getInt("r3_pr_v3_bi_s", false);

  role_3.persembe.vakit_1.bitisDakika = preferences.getInt("r3_pr_v1_bi_d", false);
  role_3.persembe.vakit_2.bitisDakika = preferences.getInt("r3_pr_v2_bi_d", false);
  role_3.persembe.vakit_3.bitisDakika = preferences.getInt("r3_pr_v3_bi_d", false);

  role_3.cuma.vakit_1.bitisSaat = preferences.getInt("r3_cu_v1_bi_s", false);
  role_3.cuma.vakit_2.bitisSaat = preferences.getInt("r3_cu_v2_bi_s", false);
  role_3.cuma.vakit_3.bitisSaat = preferences.getInt("r3_cu_v3_bi_s", false);

  role_3.cuma.vakit_1.bitisDakika = preferences.getInt("r3_cu_v1_bi_d", false);
  role_3.cuma.vakit_2.bitisDakika = preferences.getInt("r3_cu_v2_bi_d", false);
  role_3.cuma.vakit_3.bitisDakika = preferences.getInt("r3_cu_v3_bi_d", false);

  role_3.cumartesi.vakit_1.bitisSaat = preferences.getInt("r3_ct_v1_bi_s", false);
  role_3.cumartesi.vakit_2.bitisSaat = preferences.getInt("r3_ct_v2_bi_s", false);
  role_3.cumartesi.vakit_3.bitisSaat = preferences.getInt("r3_ct_v3_bi_s", false);

  role_3.cumartesi.vakit_1.bitisDakika = preferences.getInt("r3_ct_v1_bi_d", false);
  role_3.cumartesi.vakit_2.bitisDakika = preferences.getInt("r3_ct_v2_bi_d", false);
  role_3.cumartesi.vakit_3.bitisDakika = preferences.getInt("r3_ct_v3_bi_d", false);

  role_3.pazar.vakit_1.bitisSaat = preferences.getInt("r3_pz_v1_bi_s", false);
  role_3.pazar.vakit_2.bitisSaat = preferences.getInt("r3_pz_v2_bi_s", false);
  role_3.pazar.vakit_3.bitisSaat = preferences.getInt("r3_pz_v3_bi_s", false);

  role_3.pazar.vakit_1.bitisDakika = preferences.getInt("r3_pz_v1_bi_d", false);
  role_3.pazar.vakit_2.bitisDakika = preferences.getInt("r3_pz_v2_bi_d", false);
  role_3.pazar.vakit_3.bitisDakika = preferences.getInt("r3_pz_v3_bi_d", false);
}

void verileriCekRole4Baslangic()
{
  role_4.pazartesi.vakit_1.baslangicSaat = preferences.getInt("r4_pt_v1_ba_s", false);
  role_4.pazartesi.vakit_2.baslangicSaat = preferences.getInt("r4_pt_v2_ba_s", false);
  role_4.pazartesi.vakit_3.baslangicSaat = preferences.getInt("r4_pt_v3_ba_s", false);

  role_4.pazartesi.vakit_1.baslangicDakika = preferences.getInt("r4_pt_v1_ba_d", false);
  role_4.pazartesi.vakit_2.baslangicDakika = preferences.getInt("r4_pt_v2_ba_d", false);
  role_4.pazartesi.vakit_3.baslangicDakika = preferences.getInt("r4_pt_v3_ba_d", false);

  role_4.sali.vakit_1.baslangicSaat = preferences.getInt("r4_sl_v1_ba_s", false);
  role_4.sali.vakit_2.baslangicSaat = preferences.getInt("r4_sl_v2_ba_s", false);
  role_4.sali.vakit_3.baslangicSaat = preferences.getInt("r4_sl_v3_ba_s", false);

  role_4.sali.vakit_1.baslangicDakika = preferences.getInt("r4_sl_v1_ba_d", false);
  role_4.sali.vakit_2.baslangicDakika = preferences.getInt("r4_sl_v2_ba_d", false);
  role_4.sali.vakit_3.baslangicDakika = preferences.getInt("r4_sl_v3_ba_d", false);

  role_4.carsamba.vakit_1.baslangicSaat = preferences.getInt("r4_cr_v1_ba_s", false);
  role_4.carsamba.vakit_2.baslangicSaat = preferences.getInt("r4_cr_v2_ba_s", false);
  role_4.carsamba.vakit_3.baslangicSaat = preferences.getInt("r4_cr_v3_ba_s", false);

  role_4.carsamba.vakit_1.baslangicDakika = preferences.getInt("r4_cr_v1_ba_d", false);
  role_4.carsamba.vakit_2.baslangicDakika = preferences.getInt("r4_cr_v2_ba_d", false);
  role_4.carsamba.vakit_3.baslangicDakika = preferences.getInt("r4_cr_v3_ba_d", false);

  role_4.persembe.vakit_1.baslangicSaat = preferences.getInt("r4_pr_v1_ba_s", false);
  role_4.persembe.vakit_2.baslangicSaat = preferences.getInt("r4_pr_v2_ba_s", false);
  role_4.persembe.vakit_3.baslangicSaat = preferences.getInt("r4_pr_v3_ba_s", false);

  role_4.persembe.vakit_1.baslangicDakika = preferences.getInt("r4_pr_v1_ba_d", false);
  role_4.persembe.vakit_2.baslangicDakika = preferences.getInt("r4_pr_v2_ba_d", false);
  role_4.persembe.vakit_3.baslangicDakika = preferences.getInt("r4_pr_v3_ba_d", false);

  role_4.cuma.vakit_1.baslangicSaat = preferences.getInt("r4_cu_v1_ba_s", false);
  role_4.cuma.vakit_2.baslangicSaat = preferences.getInt("r4_cu_v2_ba_s", false);
  role_4.cuma.vakit_3.baslangicSaat = preferences.getInt("r4_cu_v3_ba_s", false);

  role_4.cuma.vakit_1.baslangicDakika = preferences.getInt("r4_cu_v1_ba_d", false);
  role_4.cuma.vakit_2.baslangicDakika = preferences.getInt("r4_cu_v2_ba_d", false);
  role_4.cuma.vakit_3.baslangicDakika = preferences.getInt("r4_cu_v3_ba_d", false);

  role_4.cumartesi.vakit_1.baslangicSaat = preferences.getInt("r4_ct_v1_ba_s", false);
  role_4.cumartesi.vakit_2.baslangicSaat = preferences.getInt("r4_ct_v2_ba_s", false);
  role_4.cumartesi.vakit_3.baslangicSaat = preferences.getInt("r4_ct_v3_ba_s", false);

  role_4.cumartesi.vakit_1.baslangicDakika = preferences.getInt("r4_ct_v1_ba_d", false);
  role_4.cumartesi.vakit_2.baslangicDakika = preferences.getInt("r4_ct_v2_ba_d", false);
  role_4.cumartesi.vakit_3.baslangicDakika = preferences.getInt("r4_ct_v3_ba_d", false);

  role_4.pazar.vakit_1.baslangicSaat = preferences.getInt("r4_pz_v1_ba_s", false);
  role_4.pazar.vakit_2.baslangicSaat = preferences.getInt("r4_pz_v2_ba_s", false);
  role_4.pazar.vakit_3.baslangicSaat = preferences.getInt("r4_pz_v3_ba_s", false);

  role_4.pazar.vakit_1.baslangicDakika = preferences.getInt("r4_pz_v1_ba_d", false);
  role_4.pazar.vakit_2.baslangicDakika = preferences.getInt("r4_pz_v2_ba_d", false);
  role_4.pazar.vakit_3.baslangicDakika = preferences.getInt("r4_pz_v3_ba_d", false);
}

void verileriCekRole4Bitis()
{
  role_4.pazartesi.vakit_1.bitisSaat = preferences.getInt("r4_pt_v1_bi_s", false);
  role_4.pazartesi.vakit_2.bitisSaat = preferences.getInt("r4_pt_v2_bi_s", false);
  role_4.pazartesi.vakit_3.bitisSaat = preferences.getInt("r4_pt_v3_bi_s", false);

  role_4.pazartesi.vakit_1.bitisDakika = preferences.getInt("r4_pt_v1_bi_d", false);
  role_4.pazartesi.vakit_2.bitisDakika = preferences.getInt("r4_pt_v2_bi_d", false);
  role_4.pazartesi.vakit_3.bitisDakika = preferences.getInt("r4_pt_v3_bi_d", false);

  role_4.sali.vakit_1.bitisSaat = preferences.getInt("r4_sl_v1_bi_s", false);
  role_4.sali.vakit_2.bitisSaat = preferences.getInt("r4_sl_v2_bi_s", false);
  role_4.sali.vakit_3.bitisSaat = preferences.getInt("r4_sl_v3_bi_s", false);

  role_4.sali.vakit_1.bitisDakika = preferences.getInt("r4_sl_v1_bi_d", false);
  role_4.sali.vakit_2.bitisDakika = preferences.getInt("r4_sl_v2_bi_d", false);
  role_4.sali.vakit_3.bitisDakika = preferences.getInt("r4_sl_v3_bi_d", false);

  role_4.carsamba.vakit_1.bitisSaat = preferences.getInt("r4_cr_v1_bi_s", false);
  role_4.carsamba.vakit_2.bitisSaat = preferences.getInt("r4_cr_v2_bi_s", false);
  role_4.carsamba.vakit_3.bitisSaat = preferences.getInt("r4_cr_v3_bi_s", false);

  role_4.carsamba.vakit_1.bitisDakika = preferences.getInt("r4_cr_v1_bi_d", false);
  role_4.carsamba.vakit_2.bitisDakika = preferences.getInt("r4_cr_v2_bi_d", false);
  role_4.carsamba.vakit_3.bitisDakika = preferences.getInt("r4_cr_v3_bi_d", false);

  role_4.persembe.vakit_1.bitisSaat = preferences.getInt("r4_pr_v1_bi_s", false);
  role_4.persembe.vakit_2.bitisSaat = preferences.getInt("r4_pr_v2_bi_s", false);
  role_4.persembe.vakit_3.bitisSaat = preferences.getInt("r4_pr_v3_bi_s", false);

  role_4.persembe.vakit_1.bitisDakika = preferences.getInt("r4_pr_v1_bi_d", false);
  role_4.persembe.vakit_2.bitisDakika = preferences.getInt("r4_pr_v2_bi_d", false);
  role_4.persembe.vakit_3.bitisDakika = preferences.getInt("r4_pr_v3_bi_d", false);

  role_4.cuma.vakit_1.bitisSaat = preferences.getInt("r4_cu_v1_bi_s", false);
  role_4.cuma.vakit_2.bitisSaat = preferences.getInt("r4_cu_v2_bi_s", false);
  role_4.cuma.vakit_3.bitisSaat = preferences.getInt("r4_cu_v3_bi_s", false);

  role_4.cuma.vakit_1.bitisDakika = preferences.getInt("r4_cu_v1_bi_d", false);
  role_4.cuma.vakit_2.bitisDakika = preferences.getInt("r4_cu_v2_bi_d", false);
  role_4.cuma.vakit_3.bitisDakika = preferences.getInt("r4_cu_v3_bi_d", false);

  role_4.cumartesi.vakit_1.bitisSaat = preferences.getInt("r4_ct_v1_bi_s", false);
  role_4.cumartesi.vakit_2.bitisSaat = preferences.getInt("r4_ct_v2_bi_s", false);
  role_4.cumartesi.vakit_3.bitisSaat = preferences.getInt("r4_ct_v3_bi_s", false);

  role_4.cumartesi.vakit_1.bitisDakika = preferences.getInt("r4_ct_v1_bi_d", false);
  role_4.cumartesi.vakit_2.bitisDakika = preferences.getInt("r4_ct_v2_bi_d", false);
  role_4.cumartesi.vakit_3.bitisDakika = preferences.getInt("r4_ct_v3_bi_d", false);

  role_4.pazar.vakit_1.bitisSaat = preferences.getInt("r4_pz_v1_bi_s", false);
  role_4.pazar.vakit_2.bitisSaat = preferences.getInt("r4_pz_v2_bi_s", false);
  role_4.pazar.vakit_3.bitisSaat = preferences.getInt("r4_pz_v3_bi_s", false);

  role_4.pazar.vakit_1.bitisDakika = preferences.getInt("r4_pz_v1_bi_d", false);
  role_4.pazar.vakit_2.bitisDakika = preferences.getInt("r4_pz_v2_bi_d", false);
  role_4.pazar.vakit_3.bitisDakika = preferences.getInt("r4_pz_v3_bi_d", false);
}

void tarihYazdir()
{
  if(millis()-t > sure)
  {
    Serial.print("Şuanki Tarih / Saat : ");
    Serial.print(myRTC.dayofmonth);
    Serial.print("/");
    Serial.print(myRTC.month);
    Serial.print("/");
    Serial.print(myRTC.year);
    Serial.print(" ");
    Serial.print(myRTC.hours);
    Serial.print(":");
    Serial.print(myRTC.minutes);
    Serial.print(":");
    Serial.print(myRTC.seconds);
    Serial.print("  ");
    Serial.println(daysOfWeek[myRTC.dayofweek - 1]);
    t2 = millis();
  }
}

void handleRoot()
{
  server.send(200, "text/html", SendHTML(role_1.roleStatus, role_2.roleStatus, role_3.roleStatus, role_4.roleStatus, false)); //Send web page
}

void kontrolpaneli()
{
  server.send(200, "text/html", SendKontrolSayfa(false));
}

void kontrolForm()
{
  String secilenVana = server.arg("vana");
  String secilenGun = server.arg("gun");
  String secilenButton = server.arg("submit");

  Serial.println(secilenGun);
  Serial.println(secilenVana);
  Serial.println(secilenButton);

  if (secilenButton == "GÖSTER")
  {
    if (secilenVana == "vana1")
    {
      role_1.kontrolPaneliDoldur(secilenGun);
    }

    else if (secilenVana == "vana2")
    {
      role_2.kontrolPaneliDoldur(secilenGun);
    }

    else if (secilenVana == "vana3")
    {
      role_3.kontrolPaneliDoldur(secilenGun);
    }

    else if (secilenVana == "vana4")
    {
      role_4.kontrolPaneliDoldur(secilenGun);
    }
  }

  if (secilenButton == "SIFIRLA")
  {
    if (secilenVana == "vana1")
    {
      role_1.gunsifirla(secilenGun);
      kaydetRole1Baslangic();
      kaydetRole1Bitis();
      delay(100);
    }

    else if (secilenVana == "vana2")
    {
      role_2.gunsifirla(secilenGun);
      kaydetRole2Baslangic();
      kaydetRole2Bitis();
      delay(100);
    }

    else if (secilenVana == "vana3")
    {
      role_3.gunsifirla(secilenGun);
      kaydetRole3Baslangic();
      kaydetRole3Bitis();
      delay(100);
    }

    else if (secilenVana == "vana4")
    {
      role_4.gunsifirla(secilenGun);
      kaydetRole4Baslangic();
      kaydetRole4Bitis();
      delay(100);
    }
    server.send(200, "text/html", SendKontrolSayfa(true));
  }
}

void handleForm1()
{
  role_1.formCekme();
  kaydetRole1Baslangic();
  kaydetRole1Bitis();
  delay(100);

  //Serial.println("kaydedildi");

  server.send(200, "text/html", SendHTML(role_1.roleStatus, role_2.roleStatus, role_3.roleStatus, role_4.roleStatus, true));
}

void handleForm2()
{
  role_2.formCekme();
  kaydetRole2Baslangic();
  kaydetRole2Bitis();
  delay(100);

  server.send(200, "text/html", SendHTML(role_1.roleStatus, role_2.roleStatus, role_3.roleStatus, role_4.roleStatus, true));
}

void handleForm3()
{
  role_3.formCekme();
  kaydetRole3Baslangic();
  kaydetRole3Bitis();
  delay(100);

  server.send(200, "text/html", SendHTML(role_1.roleStatus, role_2.roleStatus, role_3.roleStatus, role_4.roleStatus, true));
}

void handleForm4()
{
  role_4.formCekme();
  kaydetRole4Baslangic();
  kaydetRole4Bitis();
  delay(100);

  server.send(200, "text/html", SendHTML(role_1.roleStatus, role_2.roleStatus, role_3.roleStatus, role_4.roleStatus, true));
}

// rolelerin açıp kapanamsını loopa al

void handle_role_1_on()
{
  role_1.roleStatus = HIGH;
  Serial.println("ROLE 1 Durumu: ON");
  server.send(200, "text/html", SendHTML(role_1.roleStatus, role_2.roleStatus, role_3.roleStatus, role_4.roleStatus, false));
}

void handle_role_1_off()
{
  role_1.roleStatus = LOW;
  Serial.println("ROLE 1 Durumu: OFF");
  server.send(200, "text/html", SendHTML(role_1.roleStatus, role_2.roleStatus, role_3.roleStatus, role_4.roleStatus, false));
}

void handle_role_2_on()
{
  role_2.roleStatus = HIGH;
  Serial.println("ROLE 2 Durumu: ON");
  server.send(200, "text/html", SendHTML(role_1.roleStatus, role_2.roleStatus, role_3.roleStatus, role_4.roleStatus, false));
}

void handle_role_2_off()
{
  role_2.roleStatus = LOW;
  Serial.println("ROLE 2 Durumu: OFF");
  server.send(200, "text/html", SendHTML(role_1.roleStatus, role_2.roleStatus, role_3.roleStatus, role_4.roleStatus, false));
}

void handle_role_3_on()
{
  role_3.roleStatus = HIGH;
  Serial.println("ROLE 3 Durumu: ON");
  server.send(200, "text/html", SendHTML(role_1.roleStatus, role_2.roleStatus, role_3.roleStatus, role_4.roleStatus, false));
}

void handle_role_3_off()
{
  role_3.roleStatus = LOW;
  Serial.println("ROLE 3 Durumu: OFF");
  server.send(200, "text/html", SendHTML(role_1.roleStatus, role_2.roleStatus, role_3.roleStatus, role_4.roleStatus, false));
}

void handle_role_4_on()
{
  role_4.roleStatus = HIGH;
  Serial.println("ROLE 4 Durumu: ON");
  server.send(200, "text/html", SendHTML(role_1.roleStatus, role_2.roleStatus, role_3.roleStatus, role_4.roleStatus, false));
}

void handle_role_4_off()
{
  role_4.roleStatus = LOW;
  Serial.println("ROLE 4 Durumu: OFF");
  server.send(200, "text/html", SendHTML(role_1.roleStatus, role_2.roleStatus, role_3.roleStatus, role_4.roleStatus, false));
}

void handle_NotFound()
{
  server.send(404, "text/plain", "Sayfa Bulunamadı");
}

void mainFonk()
{
  role_1.kontrol();
  role_2.kontrol();
  role_3.kontrol();
  role_4.kontrol();
  digitalWrite(role_1.pin, role_1.roleStatus);
  digitalWrite(role_2.pin, role_2.roleStatus);
  digitalWrite(role_3.pin, role_3.roleStatus);
  digitalWrite(role_4.pin, role_4.roleStatus);
}

void durumGoster()
{
  if(millis()-t > sure)
  {
    Serial.println("Vana 1: " + String(role_1.roleStatus));
    Serial.println("Vana 2: " + String(role_2.roleStatus));
    Serial.println("Vana 3: " + String(role_3.roleStatus));
    Serial.println("Vana 4: " + String(role_4.roleStatus));
    t = millis();
  }
}

void setup()
{
  Serial.begin(115200);

  //myRTC._DS1302_start();

  tanimlama();

  pinMode(role_1.pin, OUTPUT);
  pinMode(role_2.pin, OUTPUT);
  pinMode(role_3.pin, OUTPUT);
  pinMode(role_4.pin, OUTPUT);

  preferences.begin("my_app", false);

  verileriCekRole1Baslangic();
  verileriCekRole1Bitis();
  delay(100);
  verileriCekRole2Baslangic();
  verileriCekRole2Bitis();
  delay(100);
  verileriCekRole3Baslangic();
  verileriCekRole3Bitis();
  delay(100);
  verileriCekRole4Baslangic();
  verileriCekRole4Bitis();
  delay(100);

  Serial.println("Veriler Alındı");

  WiFi.mode(WIFI_AP);
  WiFi.softAP("ESP8266", "12345678");
  Serial.println();
  Serial.print("IP Adresim=");
  Serial.println(WiFi.softAPIP());

  //myRTC.setDS1302Time(00, 52, 21, 7, 14, 8, 2021);    // saati ayarlamak için

  server.on("/", handleRoot); //Which routine to handle at root location
  server.on("/kontrolpaneli", kontrolpaneli);
  server.on("/kontrolform", kontrolForm);
  server.on("/form/1", handleForm1); //form action is handled here
  server.on("/form/2", handleForm2);
  server.on("/form/3", handleForm3);
  server.on("/form/4", handleForm4);
  server.on("/role1/on", handle_role_1_on);
  server.on("/role1/off", handle_role_1_off);
  server.on("/role2/on", handle_role_2_on);
  server.on("/role2/off", handle_role_2_off);
  server.on("/role3/on", handle_role_3_on);
  server.on("/role3/off", handle_role_3_off);
  server.on("/role4/on", handle_role_4_on);
  server.on("/role4/off", handle_role_4_off);
  server.onNotFound(handle_NotFound);

  server.begin();                        //Start server
  Serial.println("HTTP server started"); //Start server
}

void loop()
{
  myRTC.updateTime();
  server.handleClient();
  mainFonk();
  tarihYazdir();
  durumGoster();
}
