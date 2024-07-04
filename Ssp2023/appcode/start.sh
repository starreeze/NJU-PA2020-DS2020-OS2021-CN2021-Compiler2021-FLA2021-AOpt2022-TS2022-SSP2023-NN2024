export PROTOCOL_BUFFERS_PYTHON_IMPLEMENTATION=python
gcloud auth application-default set-quota-project advance-anvil-399603
gcloud iam service-accounts create speech-to-text-quickstart --project advance-anvil-399603
gcloud projects add-iam-policy-binding advance-anvil-399603 \
   --member serviceAccount:speech-to-text-quickstart@advance-anvil-399603.iam.gserviceaccount.com \
    --role roles/speech.editor

gcloud iam service-accounts keys create speech-to-text-key.json --iam-account \
   speech-to-text-quickstart@advance-anvil-399603.iam.gserviceaccount.com

export GOOGLE_APPLICATION_CREDENTIALS=speech-to-text-key.json
  
gcloud beta services identity create --service=speech.googleapis.com --project=advance-anvil-399603
gcloud projects add-iam-policy-binding neos-stt-0 --member serviceAccount:service-${project_number?}@gcp-sa-speech.iam.gserviceaccount.com --role roles/speech.serviceAgent
