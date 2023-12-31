#include "tvmainwindow.h"
#include "ui_tvmainwindow.h"
#include <QSettings>
#include <QSize>
#include <glib/gprintf.h>
#include <gst/gst.h>
#include <iostream>
#include <cstring>
#include <string>



TVMainWindow::TVMainWindow(QWidget *parent, char* filename) :
    QMainWindow(parent),
    ui(new Ui::TVMainWindow)

{	camera_caps(); // appel de camera_caps afin de créer les structure utilisant les fonction Gstreamer
    ui->setupUi(this);

    recorder = new TVGSRecorder(filename);

    //Connect handler to manage recorder started and finished events
    connect(recorder, SIGNAL(started()), this, SLOT(cb_recorder_started()));
    connect(recorder, SIGNAL(finished()), this, SLOT(cb_recorder_finished()));




//création d'une Stringlist contenant toute les options des Cameras
	QStringList Camera;
	int c;

		for (c = 0;c < nbr_cameras ; c++){

	     Camera << liste_cameras[c].nom;

			}

			ui->CameracomboBox->addItems(Camera);

			QStringList video;
			int v;

				for (v = 0;v < liste_cameras[0].nbr_resolution ; v++){

//utilisation de la structure créer par camera_caps et introduction de leur information dans les comboboxs
					QString fullresolution ( "largeur :");
					fullresolution.append( liste_cameras[0].options[v].largeur);
					fullresolution.append( ", hauteur :");
					fullresolution.append(liste_cameras[0].options[v].hauteur);
					fullresolution.append( ", framerate :");
					fullresolution.append(liste_cameras[0].options[v].framerate);


			     video << fullresolution;
					}
	       ui->videoComboBox->addItems(video); //add Sringlist items into the combobox




  // connect les 2 combobox ensemble de maniére a que a chaque selection de camera, connect envoie un signal qui fait appel a la fonction UpdateCombo
	 connect(	ui->CameracomboBox,SIGNAL(currentTextChanged(const QString &)),this,SLOT(UpdateCombo()));




    //Load last user settings
    QSettings settings(QApplication::applicationDirPath() + "/TimedVideo.ini", QSettings::IniFormat);
    int videoQuantizerSpinBox= settings.value("video/quantizer",18).toInt();
    int videoSpeedSpinBox = settings.value("video/speed", 1).toInt();
    int audioQualitySpinBox = settings.value("audio/quality", 2).toInt();
    QString optionscombobox = settings.value("Options").toString();
    QString cameracombo = settings.value("camera").toString();
    // intégrales contenant les index trouvés par findText
    int findcamera;
    int findoptions;

    findcamera = ui->CameracomboBox->findText(cameracombo); //recherche le texte de la derniére sauvegarde et retourne l'index si trouvé


    if (findcamera == -1){ // si il ne trouve rien retourne zero

    }else{
    ui->CameracomboBox->setCurrentIndex(findcamera); //set l'option afficher a l'iffichage au texte trouver par findtext
    UpdateCombo();//met a jour la seconde combobox
      findoptions = ui->videoComboBox->findText(optionscombobox);// répéte le procesus pour la seconde Combobox
      if(findoptions == -1){

      }else{
        ui->videoComboBox->setCurrentIndex(findoptions);
      }
    }


    ui->videoQuantizerSpinBox->setValue(videoQuantizerSpinBox);
    ui->videoSpeedSpinBox->setValue(videoSpeedSpinBox);
    ui->audioQualitySpinBox->setValue(audioQualitySpinBox);

}

TVMainWindow::~TVMainWindow()
{
    //Save User Settings
QSettings settings(QApplication::applicationDirPath() + "/TimedVideo.ini", QSettings::IniFormat);

      settings.setValue("camera",ui->CameracomboBox->currentText());
      settings.setValue("Options",ui->videoComboBox->currentText());
      settings.setValue("video/quantizer",ui->videoQuantizerSpinBox->value());
      settings.setValue("video/speed",ui->videoSpeedSpinBox->value());
      settings.setValue("audio/quality",ui->audioQualitySpinBox->value());


//Clean up the class member variables
      delete ui;
      delete recorder;



}


void TVMainWindow::on_startButton_clicked()
{
  //Initialize the recording player if needed


 QSize videoSize(atoi(liste_cameras[ui->CameracomboBox->currentIndex()].options[ui->videoComboBox->currentIndex()].largeur),atoi(liste_cameras[ui->CameracomboBox->currentIndex()].options[ui->videoComboBox->currentIndex()].hauteur));
 if(recorder->init_pipeline(liste_cameras[ui->CameracomboBox->currentIndex()].options[ui->videoComboBox->currentIndex()].framerateUP,liste_cameras[ui->CameracomboBox->currentIndex()].options[ui->videoComboBox->currentIndex()].framerateDOWN, videoSize, ui->videoQuantizerSpinBox->value(), ui->videoSpeedSpinBox->value(), ui->audioQualitySpinBox->value(),liste_cameras[ui->CameracomboBox->currentIndex()].path) == false)
     return;

    //Associate recorder video output the video widget (should be done each time, why ?)
    recorder->setDisplay(ui->videoWidget);

    //Start playing
    recorder->start();
}

void TVMainWindow::on_stopButton_clicked()
{
    recorder->stop();
}

void TVMainWindow::cb_recorder_started()
{
    ui->startButton->setEnabled(false);
    ui->stopButton->setEnabled(true);
}

void TVMainWindow::cb_recorder_finished()
{
    ui->startButton->setEnabled(true);
    ui->stopButton->setEnabled(false);
}

void TVMainWindow::closeEvent (QCloseEvent *event)
{
    recorder->stop();
    recorder->wait();
}


void TVMainWindow::UpdateCombo(){ //fonction qui change les items du second combobox en fonction de l'index du premmier combobox.
ui->videoComboBox->clear();
QStringList video;
int v;

	for (v = 0;v < liste_cameras[ui->CameracomboBox->currentIndex()].nbr_resolution ; v++){


		QString fullresolution ( "hauteur :");
		fullresolution.append( liste_cameras[ui->CameracomboBox->currentIndex()].options[v].hauteur);
		fullresolution.append( ", largeur :");
		fullresolution.append(liste_cameras[ui->CameracomboBox->currentIndex()].options[v].largeur);
		fullresolution.append( ", framerate :");
		fullresolution.append(liste_cameras[ui->CameracomboBox->currentIndex()].options[v].framerate);

		 video << fullresolution;
		}
	 ui->videoComboBox->addItems(video);
}
//fonction statique tirée de gst-device-inspect qui sert à itérer sur les different device que gstreamer detecte
static gboolean my_bus_func(GstBus * bus, GstMessage * message, gpointer user_data)
   {
      GstDevice *device;
      gchar *name;

      switch (GST_MESSAGE_TYPE (message)) {
        case GST_MESSAGE_DEVICE_ADDED:
          gst_message_parse_device_added (message, &device);
          name = gst_device_get_display_name (device);
          g_print("Device added: %s\n", name);
          g_free (name);
          gst_object_unref (device);
          break;
        case GST_MESSAGE_DEVICE_REMOVED:
          gst_message_parse_device_removed (message, &device);
          name = gst_device_get_display_name (device);
          g_print("Device removed: %s\n", name);
          g_free (name);
          gst_object_unref (device);
          break;
        default:
          break;
      }

      return G_SOURCE_CONTINUE;
   }
//fonction qui va creer un monitor qui ne regarde que les source vidép qui émetent en x-raw, sert à trouver et filtrer les devices connectés.
   GstDeviceMonitor *setup_raw_video_source_device_monitor (void) {
      GstDeviceMonitor *monitor;
      GstBus *bus;
      GstCaps *caps;

      monitor = gst_device_monitor_new ();

      bus = gst_device_monitor_get_bus (monitor);
      gst_bus_add_watch (bus, my_bus_func, NULL);
      gst_object_unref (bus);

      caps = gst_caps_new_empty_simple ("video/x-raw");
      gst_device_monitor_add_filter (monitor, "Video/Source", caps);
      gst_caps_unref (caps);

      gst_device_monitor_start (monitor);

      return monitor;
   }
// fonction qui quand appellé va remplir l'attribut liste-camera avec les charcteristique des differentes cameras connectée 
void TVMainWindow::camera_caps()
{
	unsigned int compteur_framerates,compteur_inspecteur1;

	int compteur_framerates2;
	GstDeviceMonitor *monitor;
	nbr_cameras = 0;

	// on crée un monitor qui va recuperer les device
	monitor = setup_raw_video_source_device_monitor();
	GList *list_gstdevice = gst_device_monitor_get_devices(monitor);
	nbr_cameras = 0;





	while(list_gstdevice != NULL){
		// on recupère le nom de la source vidéo
		gchar *nom_camera_monitorer = gst_device_get_display_name((GstDevice*)list_gstdevice->data);
		
		strcpy(liste_cameras[nbr_cameras].nom,(char *)nom_camera_monitorer);
		// on récupère  son path
		GstStructure *test_property = gst_device_get_properties((GstDevice*)list_gstdevice->data);
		gchar *test_property_char = gst_structure_to_string(test_property);

		strcpy(liste_cameras[nbr_cameras].path,gst_structure_get_string(test_property,"device.path") );
		// on prend ensuite ses capabilités dans un gst-caps qu'on va traiter plus loins
		GstCaps *Caps_current = gst_device_get_caps((GstDevice*)list_gstdevice->data);
		guint size_caps_current = gst_caps_get_size(Caps_current);
		compteur_framerates2 = 0;
		
		// on va ensuite iterer sur chaque combinaison de hauteur largeur
		for(compteur_inspecteur1 = 0; compteur_inspecteur1 < size_caps_current;compteur_inspecteur1 ++){
			GstStructure *Bout_de_Caps = gst_caps_get_structure(Caps_current,compteur_inspecteur1);
			GType type_bout_de_caps  = gst_structure_get_field_type(Bout_de_Caps,"framerate");
			// si plusieurs framerate pour un meme combo largeur hauteur
			if(strcmp("GstValueList",g_type_name(type_bout_de_caps)) == 0){
				guint taille_liste_framerate = gst_value_list_get_size(gst_structure_get_value(Bout_de_Caps,"framerate"));
				for(compteur_framerates = 0;compteur_framerates < taille_liste_framerate;compteur_framerates++){
					const GValue *GValue_framerate =gst_structure_get_value(Bout_de_Caps,"framerate");
					int denominateur = gst_value_get_fraction_denominator(gst_value_list_get_value(GValue_framerate,compteur_framerates));
					int numereteur = gst_value_get_fraction_numerator(gst_value_list_get_value(GValue_framerate,compteur_framerates));
					liste_cameras[nbr_cameras].options[compteur_framerates2].framerateDOWN = denominateur;
					liste_cameras[nbr_cameras].options[compteur_framerates2].framerateUP = numereteur;
					sprintf(liste_cameras[nbr_cameras].options[compteur_framerates2].framerate,"%i/%i",numereteur,denominateur);
					const GValue *GValue_largeur = gst_structure_get_value(Bout_de_Caps,"width");
					sprintf(liste_cameras[nbr_cameras].options[compteur_framerates2].largeur,"%i",g_value_get_int(GValue_largeur));
					const GValue *GValue_hauteur = gst_structure_get_value(Bout_de_Caps,"height");
					sprintf(liste_cameras[nbr_cameras].options[compteur_framerates2].hauteur,"%i",g_value_get_int(GValue_hauteur));
					compteur_framerates2++;
					}
				}
			//sinon on le prend directement
			if(strcmp("GstFraction",g_type_name(type_bout_de_caps)) == 0){
				const GValue *GValue_framerate =gst_structure_get_value(Bout_de_Caps,"framerate");
				int denominateur = gst_value_get_fraction_denominator(GValue_framerate);
				int numereteur = gst_value_get_fraction_numerator(GValue_framerate);
				liste_cameras[nbr_cameras].options[compteur_framerates2].framerateDOWN = denominateur;
				liste_cameras[nbr_cameras].options[compteur_framerates2].framerateUP = numereteur;
				sprintf(liste_cameras[nbr_cameras].options[compteur_framerates2].framerate,"%i/%i",numereteur,denominateur);
				const GValue *GValue_largeur = gst_structure_get_value(Bout_de_Caps,"width");
				sprintf(liste_cameras[nbr_cameras].options[compteur_framerates2].largeur,"%i",g_value_get_int(GValue_largeur));
				const GValue *GValue_hauteur = gst_structure_get_value(Bout_de_Caps,"height");
				sprintf(liste_cameras[nbr_cameras].options[compteur_framerates2].hauteur,"%i",g_value_get_int(GValue_hauteur));
				compteur_framerates2++;
				}
		}
		liste_cameras[nbr_cameras].nbr_resolution = compteur_framerates2;
		list_gstdevice = list_gstdevice->next;
		nbr_cameras ++;
	}
		//uncomment here to test things.
/*
	int i,j;
	printf("Nombre camera: %i\n",nbr_cameras);
	for (i = 0;i < compteur_device+1 ; i++){

		printf("i = %i\n",i);
		printf("nom: %s\n",liste_cameras[i].nom);
		printf("path :%s\n",liste_cameras[i].path);
		printf("nbr_res:%i\n",liste_cameras[i].nbr_resolution);
		for (j = 0;j < 100; j++) {
			printf("j = %i\n",j);
			printf("hauteur: %s\n",liste_cameras[i].options[j].hauteur);
			printf("largeur: %s\n",liste_cameras[i].options[j].largeur);
			printf("framerate: %s\n",liste_cameras[i].options[j].framerate);
			}
	}
*/


}
