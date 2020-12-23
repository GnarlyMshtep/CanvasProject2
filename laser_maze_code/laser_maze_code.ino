#define RdirPin 9
#define RstepPin 8
#define LdirPin 11
#define LstepPin 10
const int stepsPerRevolution = 6400;
const int LED = 12;
int incomingByte = 0;
int quantity = 3000;
int Time = 400;
double spool_radius = 28; //mm, check CAD drawing
double machine_width = 346; //mm
double machine_height = 231; //mm
double load_height = 35; //mm
double x_Position = 165; //initial x mm; must be physically measured

//one structure that carries 2 variables
struct vector {
  int sL_step;
  int sR_step;
};

void process_vector(vector box) {
  if (abs(box.sL_step) < abs(box.sR_step)) {
    int quotient;
    int remainder;
    quotient = abs(box.sR_step)/abs(box.sL_step);
    remainder = abs(box.sR_step)%abs(box.sL_step);
    for (int j = 0; j < abs(box.sL_step); j++) {
      Left_rotate(box.sL_step < 0);
      if (remainder-- > 0) Right_rotate(box.sR_step < 0);
      for (int k = 0; k < quotient; k++) {
        Right_rotate(box.sR_step < 0);
      }
    }
  }
  if (abs(box.sL_step) > abs(box.sR_step)) {
    int quotient;
    int remainder;
    quotient = abs(box.sL_step)/abs(box.sR_step);
    remainder = abs(box.sL_step)%abs(box.sR_step);
    for (int j = 0; j < abs(box.sR_step); j++) {
      Right_rotate(box.sR_step < 0);
      if (remainder-- > 0) Left_rotate(box.sL_step < 0);
      for (int k = 0; k < quotient; k++) {
        Left_rotate(box.sL_step < 0);
      }
    }
  }
}

class load {
  private:
  //attributes
  double x,y, SL, SR, step_distance;
  // the following three private methods are for housekeeping
  double updateSL(float dx, float dy) {
    return sqrt(pow(x + dx,2)+pow(y + dy,2));
  }
  double updateSR(float dx, float dy) {
    return sqrt(pow(machine_width - (x+dx),2)+pow(y+dy,2));
  }
  double setStep_distance(float radius) {
    return (2*PI*radius)/stepsPerRevolution;
  }
  public: 
  load(double initial_x, double initial_y, double radius) : //constructor for load object
    x(initial_x), 
    y(initial_y), 
    step_distance(setStep_distance(radius)),
    SL(updateSL(0,0)),
    SR(updateSR(0,0)) {}
    
  //motion code
  vector motion(float delta_x, float delta_y) { //input in mm
    Serial.println("Calling load function motion!");  
    double temp_SLF = updateSL(delta_x,delta_y);
    Serial.print("temp_SLF: ");
    Serial.println(temp_SLF,7);
    double temp_SRF = updateSR(delta_x,delta_y);
    Serial.print("temp_SRF: ");
    Serial.println(temp_SRF,7);
    int temp_SL_step = (temp_SLF-SL)/step_distance;
    Serial.print("temp_SL_step: ");
    Serial.println(temp_SL_step);
    int temp_SR_step = (temp_SRF-SR)/step_distance;
    Serial.print("temp_SR_step: ");
    Serial.println(temp_SR_step);
    vector strap_length{temp_SL_step,temp_SR_step};
    SL += temp_SL_step*step_distance; 
    SR += temp_SR_step*step_distance; 
    Serial.println("BEFORE");
    Serial.print("x: ");
    Serial.println(x,7);
    Serial.print("y: ");
    Serial.println(y,7);
    x = (pow(machine_width,2)+pow(SL,2)-pow(SR,2))/(2*machine_width); //derived equation in notebook
    y = sqrt(pow(SL,2)-pow(x,2));
    Serial.println("AFTER");
    Serial.print("x: ");
    Serial.println(x,7);
    Serial.print("y: ");
    Serial.println(y,7);
    Serial.print("SL: ");
    Serial.println(SL,7);
    Serial.print("SR: ");
    Serial.println(SR,7);
    Serial.println("Just finished calling load function motion!"); 
    return strap_length;
  }

  vector reset() {
    Serial.print("x_Position-x: ");
    Serial.println(x_Position-x);
    Serial.print("machine_height-load_height-y-10: ");
    Serial.println(machine_height-load_height-y-10);
    vector box = motion(x_Position-x,machine_height-load_height-y-10);
    return box;
  }
};

load laser(x_Position,machine_height-load_height,spool_radius);
void setup() {
  // put your setup code here, to run once:
  pinMode(RstepPin, OUTPUT);
  pinMode(RdirPin, OUTPUT);
  pinMode(LstepPin, OUTPUT);
  pinMode(LdirPin, OUTPUT);
  pinMode(LED, OUTPUT);
  Serial.begin(9600);
  Serial.println("Booting Up!");
  digitalWrite(RstepPin, HIGH);
  digitalWrite(LstepPin, HIGH);
  analogWrite(LED, 150);
  vector container = laser.motion(0, -10);
  process_vector(container);
  delay(1000);
}

void loop() {
//  for (int i = 0; i < 6; i++) {
//    vector container;
//    if (i < 3) container = laser.motion(0, -30);
//    else container = laser.motion(0, 30);
//    Serial.print("sL_step: ");
//    Serial.println(container.sL_step);
//    Serial.print("sR_step: ");
//    Serial.println(container.sR_step);
//    process_vector(container);
//    Serial.print("Iteration ");
//    Serial.print(i+1);
//    Serial.println(" done!");
//    delay(1000);
//  }
//  vector container = laser.motion(0, 10);
//  process_vector(container);
//  exit(0);
  if (Serial.available() > 0) {
      incomingByte = Serial.read();
      Serial.println(incomingByte);
      switch (incomingByte) {
        vector container;
        case 49: //1
          container = laser.motion(0, -30);
          process_vector(container);
          break;
        case 50: //2
          container = laser.motion(0, 30);
          process_vector(container);
          break;
        case 51: //3
          container = laser.motion(30, 0);
          process_vector(container);
          break;
        case 52: //4
          container = laser.motion(-30, 0);
          process_vector(container);
          break;
        case 53: //5
          container = laser.reset();
          process_vector(container);
          break;
      }
   }

}

void Right_rotate(bool state) { //if sR_step is negaitve, use CC to shorten. Else, C
  if (state) digitalWrite(RdirPin, HIGH); //when R is HIGH, CC - TRUE
  else digitalWrite(RdirPin, LOW); //when R is LOW, C - FALSE
  digitalWrite(LstepPin, HIGH);
  digitalWrite(RstepPin, LOW);
  delayMicroseconds(Time);
  digitalWrite(RstepPin, HIGH);
  delayMicroseconds(Time);
}

void Left_rotate(bool state) { //if sL_step is negaitve, use C to shorten. Else, CC
  if (state) digitalWrite(LdirPin, LOW); //when L is LOW, C - FALSE
  else digitalWrite(LdirPin, HIGH); //when L is HIGH, CC - TRUE
  digitalWrite(RstepPin, HIGH);
  digitalWrite(LstepPin, LOW);
  delayMicroseconds(Time);
  digitalWrite(LstepPin, HIGH);
  delayMicroseconds(Time);
}
