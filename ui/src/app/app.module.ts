import {NgModule} from '@angular/core';
import {BrowserModule} from '@angular/platform-browser';

import {AppComponent} from './app.component';
import {BrowserAnimationsModule} from '@angular/platform-browser/animations';
import {MatTableModule} from "@angular/material/table";
import {BeaconAtReceiverPipe} from './pipes/beacon-at-receiver.pipe';
import {BeaconDataComponent} from './components/beacon-data/beacon-data.component';
import {TimeElapsedService} from "./services/timeElapsed/time-elapsed.service";
import { NanoToSecondsPipe } from './pipes/nano-to-seconds.pipe';
import { InPipe } from './pipes/in.pipe';
import { IsReceiverInactivePipe } from './pipes/is-receiver-inactive.pipe';
import {NgxIndexedDBModule} from "ngx-indexed-db";
import {dbConfig} from "./DatabaseConfig";
import {MatButtonModule} from "@angular/material/button";
import { ReceiverAliasPipe } from './pipes/receiver-alias.pipe';

@NgModule({
  declarations: [
    AppComponent,
    BeaconAtReceiverPipe,
    BeaconDataComponent,
    NanoToSecondsPipe,
    InPipe,
    IsReceiverInactivePipe,
    ReceiverAliasPipe,
  ],
  imports: [
    BrowserModule,
    BrowserAnimationsModule,
    MatTableModule,
    NgxIndexedDBModule.forRoot(dbConfig),
    MatButtonModule
  ],
  providers: [],
  bootstrap: [AppComponent]
})
export class AppModule {
}
