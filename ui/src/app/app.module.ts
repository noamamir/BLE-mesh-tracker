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
import { AliasNamesPipe } from './pipes/receiver-alias.pipe';
import {HttpClientModule} from "@angular/common/http";
import {MatInputModule} from "@angular/material/input";
import {MatIconModule} from "@angular/material/icon";
import { AliasNameInputComponent } from './components/alias-name-input/alias-name-input.component';

@NgModule({
  declarations: [
    AppComponent,
    BeaconAtReceiverPipe,
    BeaconDataComponent,
    NanoToSecondsPipe,
    InPipe,
    IsReceiverInactivePipe,
    AliasNamesPipe,
    AliasNameInputComponent,
  ],
  imports: [
    BrowserModule,
    BrowserAnimationsModule,
    MatTableModule,
    NgxIndexedDBModule.forRoot(dbConfig),
    MatButtonModule,
    HttpClientModule,
    MatInputModule,
    MatIconModule
  ],
  providers: [],
  bootstrap: [AppComponent]
})
export class AppModule {
}
